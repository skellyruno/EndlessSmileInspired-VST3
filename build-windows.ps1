$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot
cmake -S . -B build -A x64
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed" }
cmake --build build --config Release --target EndlessSmileInspired_VST3 --parallel
if ($LASTEXITCODE -ne 0) { throw "VST3 compilation failed" }
$binary = "build\EndlessSmileInspired_artefacts\Release\VST3\EndlessSmileInspired.vst3\Contents\x86_64-win\EndlessSmileInspired.vst3"
if (!(Test-Path $binary -PathType Leaf)) { throw "Build returned success but no VST3 binary: $binary" }
Write-Host "Verified VST3 binary: $binary"
