$ErrorActionPreference = "Stop"
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target EndlessSmileInspired_VST3 EndlessSmileInspired_Standalone
Write-Host "Build complete. Inspect build\EndlessSmileInspired_artefacts\Release"
