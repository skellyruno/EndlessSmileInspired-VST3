#!/usr/bin/env python3
"""Static compatibility guard against the original EndlessSmileInspired VST3 host identity."""
from pathlib import Path
import json
import re

root = Path(__file__).resolve().parent.parent
cmake = (root / "CMakeLists.txt").read_text()
processor = (root / "Source/PluginProcessor.cpp").read_text()
identity = json.loads((root / "release/vst3_identity.json").read_text(encoding="utf-8"))

for stable in (
    'BUNDLE_ID "audio.skelly.endlesssmileinspired"',
    'PLUGIN_MANUFACTURER_CODE Skly',
    'PLUGIN_CODE EsI1',
    'PRODUCT_NAME "EndlessSmileInspired"',
    'FORMATS VST3',
):
    assert stable in cmake, f"Broken host/plugin compatibility: {stable}"

version = re.search(r"project\(\s*EndlessSmileInspired\s+VERSION\s+([^\s)]+)", cmake)
assert version and version.group(1) == identity["version"], "Release identity version differs from CMake"
assert identity["productName"] == "EndlessSmileInspired", "Protected VST3 product name changed"
assert identity["bundleId"] == "audio.skelly.endlesssmileinspired", "Protected bundle ID changed"
assert identity["manufacturerCode"] == "Skly" and identity["pluginCode"] == "EsI1", "Protected VST3 codes changed"
assert len(identity["classIds"]) == 3 and len(set(identity["classIds"])) == 3, "VST3 class IDs must remain unique"
assert all(re.fullmatch(r"[0-9A-Fa-f]{32}", value) for value in identity["classIds"]), "Malformed VST3 class ID"

params = re.findall(r'ParameterID\s*\{\s*"([^"]+)"\s*,\s*(\d+)\s*\}', processor)
assert params[:3] == [('intensity', '1'), ('preset', '1'), ('mix', '1')], f"Legacy parameter IDs/order changed: {params}"
assert params[3:] == [('sync', '1'), ('corruption', '1')], f"Unexpected new parameter IDs/order: {params}"

presets = re.search(r'StringArray\s*\{(.*?)\}\s*,\s*0\s*\)\s*\)', processor, re.S)
assert presets, 'Preset choices not found'
names = re.findall(r'"([^"]+)"', presets.group(1))
assert names[:7] == [
    'Fist in the Air', 'Helium Jumping', 'No Limit', 'Face Melting Joy',
    'Smack You in the Face', 'Broken Teeth', 'Dada Dynamite',
], f"Legacy preset positions changed: {names[:7]}"
assert len(names) == 16, f"Expected 16 presets, found {len(names)}"
print('Compatibility contract passed: host identity, 5 automation parameters, 16 preset positions, 3 VST3 class IDs')
