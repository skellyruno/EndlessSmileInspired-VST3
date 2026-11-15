#!/usr/bin/env bash
# Offline DSP regression tests against a signature-compatible JUCE stand-in.
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
cp "$here/../Source/PluginProcessor.h" "$here/../Source/PluginProcessor.cpp" "$tmp/"
cp "$here/mock/PluginEditor.h" "$tmp/"
g++ -std=c++17 -O2 -Wall -Wextra -Wshadow -I"$here/mock" -I"$tmp" "$tmp/PluginProcessor.cpp" "$here/transport_test.cpp" -o "$tmp/transport_test"
"$tmp/transport_test"
g++ -std=c++17 -O2 -Wall -Wextra -Wshadow -I"$here/mock" -I"$tmp" "$tmp/PluginProcessor.cpp" "$here/dsp_test.cpp" -o "$tmp/dsp_test"
"$tmp/dsp_test"
