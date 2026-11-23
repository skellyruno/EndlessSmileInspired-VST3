#!/usr/bin/env python3
"""Validate and stage a Windows VST3 preview without nesting a second ZIP."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
import struct
from pathlib import Path
from typing import Any


def strip_trailing_commas(source: str) -> str:
    """Remove JUCE's trailing commas while leaving quoted string contents intact."""
    output: list[str] = []
    in_string = False
    escaped = False
    index = 0

    while index < len(source):
        char = source[index]
        if in_string:
            output.append(char)
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == '"':
                in_string = False
            index += 1
            continue

        if char == '"':
            in_string = True
            output.append(char)
            index += 1
            continue

        if char == ",":
            lookahead = index + 1
            while lookahead < len(source) and source[lookahead].isspace():
                lookahead += 1
            if lookahead < len(source) and source[lookahead] in "]}":
                index += 1
                continue

        output.append(char)
        index += 1

    if in_string:
        raise ValueError("moduleinfo.json contains an unterminated string")
    return "".join(output)


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def read_pe64_machine(path: Path) -> tuple[int, int]:
    with path.open("rb") as stream:
        dos_header = stream.read(64)
        if len(dos_header) < 64 or dos_header[:2] != b"MZ":
            raise ValueError(f"Not a Windows PE binary (missing MZ header): {path}")
        pe_offset = struct.unpack_from("<I", dos_header, 0x3C)[0]
        if pe_offset < 64:
            raise ValueError(f"Invalid PE header offset: {pe_offset}")
        stream.seek(pe_offset)
        pe_header = stream.read(26)

    if len(pe_header) < 26 or pe_header[:4] != b"PE\0\0":
        raise ValueError(f"Missing PE signature: {path}")
    machine = struct.unpack_from("<H", pe_header, 4)[0]
    optional_magic = struct.unpack_from("<H", pe_header, 24)[0]
    if machine != 0x8664 or optional_magic != 0x20B:
        raise ValueError(
            f"Expected AMD64 PE32+ (machine 0x8664, optional 0x20B); "
            f"got machine 0x{machine:04X}, optional 0x{optional_magic:04X}"
        )
    return machine, optional_magic


def load_identity(repo_root: Path) -> dict[str, Any]:
    identity_path = repo_root / "release" / "vst3_identity.json"
    identity = json.loads(identity_path.read_text(encoding="utf-8"))
    required = (
        "productName", "displayName", "vendor", "version", "bundleId",
        "manufacturerCode", "pluginCode", "classIds",
    )
    missing = [key for key in required if not identity.get(key)]
    if missing:
        raise ValueError(f"Release identity is missing required fields: {missing}")
    class_ids = identity["classIds"]
    if len(class_ids) != 3 or len(set(class_ids)) != 3:
        raise ValueError("Release identity must contain exactly three unique VST3 class IDs")
    if any(not re.fullmatch(r"[0-9A-Fa-f]{32}", value) for value in class_ids):
        raise ValueError("Release identity contains a malformed VST3 class ID")
    return identity


def read_cmake_version(repo_root: Path, identity: dict[str, Any]) -> str:
    cmake = (repo_root / "CMakeLists.txt").read_text(encoding="utf-8")
    match = re.search(
        rf"project\(\s*EndlessSmileInspired\s+VERSION\s+([^\s)]+)",
        cmake,
        re.IGNORECASE,
    )
    if not match:
        raise ValueError("Could not read the plugin version from CMakeLists.txt")
    version = match.group(1)
    if version != identity["version"]:
        raise ValueError(
            f"Release identity version {identity['version']} does not match CMake version {version}"
        )
    return version


def parse_moduleinfo(path: Path) -> dict[str, Any]:
    raw = path.read_text(encoding="utf-8-sig")
    try:
        parsed = json.loads(strip_trailing_commas(raw))
    except json.JSONDecodeError as error:
        raise ValueError(f"moduleinfo.json is not valid JSON after normalization: {error}") from error
    if not isinstance(parsed, dict):
        raise ValueError("moduleinfo.json root must be an object")
    return parsed


def verify_bundle(
    bundle: Path,
    vst3_root: Path,
    identity: dict[str, Any],
    version: str,
) -> tuple[Path, dict[str, Any], int, int]:
    root = vst3_root.resolve(strict=True)
    resolved_bundle = bundle.resolve(strict=True)
    expected_bundle_name = identity["productName"] + ".vst3"
    if resolved_bundle.parent != root or resolved_bundle.name != expected_bundle_name:
        raise ValueError(f"Expected exactly {root / expected_bundle_name}")

    root_bundles = [
        item for item in root.iterdir()
        if item.is_dir() and item.suffix.lower() == ".vst3"
    ]
    if len(root_bundles) != 1 or root_bundles[0].resolve() != resolved_bundle:
        raise ValueError(
            f"Expected exactly one VST3 bundle in the build output; found {len(root_bundles)}"
        )

    binary = resolved_bundle / "Contents" / "x86_64-win" / expected_bundle_name
    if not binary.is_file():
        raise ValueError(f"Missing Windows x64 VST3 module: {binary}")
    module_files = [item for item in resolved_bundle.rglob("*.vst3") if item.is_file()]
    if len(module_files) != 1 or module_files[0].resolve() != binary.resolve():
        raise ValueError(f"Expected exactly one VST3 module binary; found {len(module_files)}")
    if any(item.is_file() and item.suffix.lower() == ".exe" for item in resolved_bundle.rglob("*")):
        raise ValueError("Standalone executable found inside the VST3 preview bundle")

    machine, optional_magic = read_pe64_machine(binary)
    moduleinfo_path = resolved_bundle / "Contents" / "Resources" / "moduleinfo.json"
    if not moduleinfo_path.is_file():
        raise ValueError(f"Missing VST3 module metadata: {moduleinfo_path}")
    moduleinfo = parse_moduleinfo(moduleinfo_path)

    if moduleinfo.get("Name") != identity["productName"]:
        raise ValueError("VST3 module name does not match the protected host identity")
    if moduleinfo.get("Version") != version:
        raise ValueError("VST3 module version does not match CMakeLists.txt")
    factory = moduleinfo.get("Factory Info")
    if not isinstance(factory, dict) or factory.get("Vendor") != identity["vendor"]:
        raise ValueError("VST3 vendor metadata does not match the protected identity")

    classes = moduleinfo.get("Classes")
    if not isinstance(classes, list) or len(classes) != 3:
        raise ValueError("Expected exactly three VST3 module classes")
    class_ids = [item.get("CID") for item in classes if isinstance(item, dict)]
    if len(class_ids) != 3 or len(set(class_ids)) != 3:
        raise ValueError("VST3 module metadata contains missing or duplicate class IDs")
    if set(class_ids) != set(identity["classIds"]):
        raise ValueError("VST3 class IDs differ from the protected release identity")
    for item in classes:
        if item.get("Name") != identity["productName"]:
            raise ValueError("A VST3 class name differs from the protected host identity")
        if item.get("Vendor") != identity["vendor"] or item.get("Version") != version:
            raise ValueError("VST3 class vendor/version differs from the protected identity")

    return binary, moduleinfo, machine, optional_magic


def prepare_release_package(
    bundle: Path,
    vst3_root: Path,
    output_dir: Path,
    repo_root: Path,
    source_commit: str,
    workflow_run_id: str,
    branch: str,
) -> dict[str, Any]:
    identity = load_identity(repo_root)
    version = read_cmake_version(repo_root, identity)
    source_bundle = bundle.resolve(strict=True)
    source_binary, moduleinfo, machine, optional_magic = verify_bundle(
        source_bundle, vst3_root, identity, version
    )

    output_dir = output_dir.resolve()
    output_dir.mkdir(parents=True, exist_ok=True)
    if any(output_dir.iterdir()):
        raise ValueError(f"Output directory must be empty: {output_dir}")

    staged_bundle = output_dir / source_bundle.name
    shutil.copytree(source_bundle, staged_bundle, symlinks=False)
    staged_moduleinfo = staged_bundle / "Contents" / "Resources" / "moduleinfo.json"
    staged_moduleinfo.write_text(
        json.dumps(moduleinfo, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    staged_binary, normalized_metadata, _, _ = verify_bundle(
        staged_bundle, output_dir, identity, version
    )

    files = []
    for path in sorted(item for item in staged_bundle.rglob("*") if item.is_file()):
        files.append({
            "path": path.relative_to(output_dir).as_posix(),
            "sizeBytes": path.stat().st_size,
            "sha256": sha256_file(path),
        })

    factory = normalized_metadata.get("Factory Info", {})
    commercial_blockers = [
        "FL Studio scan/load, routing, automation and state recall have not been accepted on the owner machine.",
        "Real musical A/B listening and target-machine CPU/latency acceptance remain pending.",
        "JUCE licensing and third-party attribution clearance has not been recorded as approved.",
        "Commercial pricing, trial terms and distribution approval remain owner decisions.",
    ]
    if not factory.get("URL"):
        commercial_blockers.append("The VST3 vendor URL is not configured.")
    if not factory.get("E-Mail"):
        commercial_blockers.append("The VST3 support email is not configured.")

    manifest = {
        "schemaVersion": 1,
        "displayName": identity["displayName"],
        "pluginProductName": identity["productName"],
        "version": version,
        "format": "VST3",
        "platform": "Windows x64",
        "binaryArchitecture": "AMD64 PE32+",
        "previewOnly": True,
        "commercialReleaseApproved": False,
        "distributionScope": "public GitHub Actions preview artifact; not a private customer download",
        "artifactRetentionDays": 7,
        "commercialBlockers": commercial_blockers,
        "source": {
            "commit": source_commit,
            "branch": branch,
            "workflowRunId": workflow_run_id,
        },
        "hostIdentity": {
            "bundleId": identity["bundleId"],
            "manufacturerCode": identity["manufacturerCode"],
            "pluginCode": identity["pluginCode"],
            "classIds": identity["classIds"],
        },
        "binary": {
            "path": staged_binary.relative_to(output_dir).as_posix(),
            "machine": f"0x{machine:04X}",
            "optionalHeaderMagic": f"0x{optional_magic:04X}",
            "sizeBytes": staged_binary.stat().st_size,
            "sha256": sha256_file(staged_binary),
        },
        "files": files,
    }

    (output_dir / "release-manifest.json").write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    checksum_lines = [
        f"{item['sha256']}  {item['path']}"
        for item in files
    ]
    (output_dir / "SHA256SUMS.txt").write_text(
        "\n".join(checksum_lines) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    return manifest


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bundle", type=Path, required=True)
    parser.add_argument("--vst3-root", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--repo-root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--source-commit", required=True)
    parser.add_argument("--workflow-run-id", required=True)
    parser.add_argument("--branch", required=True)
    args = parser.parse_args()

    try:
        manifest = prepare_release_package(
            args.bundle,
            args.vst3_root,
            args.output_dir,
            args.repo_root,
            args.source_commit,
            args.workflow_run_id,
            args.branch,
        )
    except (OSError, ValueError, json.JSONDecodeError) as error:
        parser.error(str(error))

    print(json.dumps({
        "status": "PASS",
        "bundle": manifest["pluginProductName"] + ".vst3",
        "version": manifest["version"],
        "binarySha256": manifest["binary"]["sha256"],
        "files": len(manifest["files"]),
        "previewOnly": manifest["previewOnly"],
    }, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
