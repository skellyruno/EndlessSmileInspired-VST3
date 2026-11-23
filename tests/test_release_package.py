#!/usr/bin/env python3
"""Regression tests for the Windows VST3 preview packager."""

import json
import struct
import tempfile
import unittest
from pathlib import Path

from scripts.prepare_vst3_preview import (
    prepare_release_package,
    strip_trailing_commas,
)

ROOT = Path(__file__).resolve().parents[1]
IDENTITY = json.loads((ROOT / "release" / "vst3_identity.json").read_text(encoding="utf-8"))


def write_fake_pe(path: Path, machine: int = 0x8664, optional_magic: int = 0x20B) -> None:
    data = bytearray(512)
    data[:2] = b"MZ"
    struct.pack_into("<I", data, 0x3C, 0x80)
    data[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<H", data, 0x84, machine)
    struct.pack_into("<H", data, 0x98, optional_magic)
    path.write_bytes(data)


def make_fixture(root: Path) -> tuple[Path, Path, Path]:
    vst3_root = root / "VST3"
    bundle = vst3_root / "Skell Smile.vst3"
    binary = bundle / "Contents" / "x86_64-win" / "Skell Smile.vst3"
    metadata = bundle / "Contents" / "Resources" / "moduleinfo.json"
    binary.parent.mkdir(parents=True)
    metadata.parent.mkdir(parents=True)
    write_fake_pe(binary)

    classes = []
    for cid in IDENTITY["classIds"]:
        classes.append({
            "CID": cid,
            "Name": IDENTITY["productName"],
            "Vendor": IDENTITY["vendor"],
            "Version": IDENTITY["version"],
            "Category": "Audio Module Class",
        })
    moduleinfo = {
        "Name": IDENTITY["productName"],
        "Version": IDENTITY["version"],
        "Factory Info": {
            "Vendor": IDENTITY["vendor"],
            "URL": "",
            "E-Mail": "",
            "Flags": {"Unicode": True,},
        },
        "Classes": classes,
    }
    text = json.dumps(moduleinfo, indent=2)
    text = text.replace('"Unicode": true', '"Unicode": true,')
    metadata.write_text(text, encoding="utf-8")
    return vst3_root, bundle, binary


class ReleasePackageTests(unittest.TestCase):
    def test_trailing_comma_normalizer_preserves_string_content(self) -> None:
        source = '{"note":"literal ,} and \\"quote\\"", "values":[1,2,],}'
        normalized = strip_trailing_commas(source)
        self.assertEqual(json.loads(normalized), {"note": 'literal ,} and "quote"', "values": [1, 2]})

    def test_stages_flat_bundle_with_hashes_and_preview_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            vst3_root, bundle, _ = make_fixture(root)
            output = root / "artifacts"
            manifest = prepare_release_package(
                bundle, vst3_root, output, ROOT,
                source_commit="a" * 40,
                workflow_run_id="12345",
                branch="main",
            )
            self.assertTrue((output / "Skell Smile.vst3").is_dir())
            self.assertTrue((output / "SHA256SUMS.txt").is_file())
            self.assertTrue((output / "release-manifest.json").is_file())
            self.assertFalse(any(output.rglob("*.zip")))
            normalized = json.loads(
                (output / "Skell Smile.vst3/Contents/Resources/moduleinfo.json").read_text()
            )
            self.assertEqual(normalized["Name"], "Skell Smile")
            self.assertTrue(manifest["previewOnly"])
            self.assertFalse(manifest["commercialReleaseApproved"])
            self.assertEqual(manifest["source"]["workflowRunId"], "12345")
            self.assertEqual(len(manifest["files"]), 2)
            checksums = (output / "SHA256SUMS.txt").read_text()
            self.assertIn(manifest["binary"]["sha256"], checksums)

    def test_rejects_non_amd64_binary(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            vst3_root, bundle, binary = make_fixture(root)
            write_fake_pe(binary, machine=0x014C)
            with self.assertRaisesRegex(ValueError, r"Expected AMD64 PE32\+"):
                prepare_release_package(
                    bundle, vst3_root, root / "artifacts", ROOT,
                    source_commit="a" * 40, workflow_run_id="12345", branch="main",
                )

    def test_rejects_duplicate_vst3_bundles(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            vst3_root, bundle, _ = make_fixture(root)
            (vst3_root / "Unexpected.vst3").mkdir()
            with self.assertRaisesRegex(ValueError, "exactly one VST3 bundle"):
                prepare_release_package(
                    bundle, vst3_root, root / "artifacts", ROOT,
                    source_commit="a" * 40, workflow_run_id="12345", branch="main",
                )

    def test_rejects_duplicate_class_ids(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            vst3_root, bundle, _ = make_fixture(root)
            metadata = bundle / "Contents" / "Resources" / "moduleinfo.json"
            raw = metadata.read_text(encoding="utf-8")
            raw = raw.replace(IDENTITY["classIds"][1], IDENTITY["classIds"][0])
            metadata.write_text(raw, encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "duplicate class IDs"):
                prepare_release_package(
                    bundle, vst3_root, root / "artifacts", ROOT,
                    source_commit="a" * 40, workflow_run_id="12345", branch="main",
                )


if __name__ == "__main__":
    unittest.main()
