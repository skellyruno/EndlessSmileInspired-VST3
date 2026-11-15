#!/usr/bin/env bash
# Type-checks PluginEditor.cpp against tests/mock_ui (signature check only; does not render).
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
g++ -std=c++17 -fsyntax-only -Wall -Wextra -Wshadow -I"$here/mock_ui" -I"$here/mock" -I"$here/../Source" "$here/../Source/PluginEditor.cpp"
echo "UI type-check clean"
