$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

$buildDir = Join-Path $PSScriptRoot "build"
cmake -S . -B $buildDir -A x64
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed" }
cmake --build $buildDir --config Release --target EndlessSmileInspired_VST3 --parallel
if ($LASTEXITCODE -ne 0) { throw "VST3 compilation failed" }

$vst3Root = Join-Path $buildDir "EndlessSmileInspired_artefacts\Release\VST3"
$bundle = Join-Path $vst3Root "EndlessSmileInspired.vst3"
$binary = Join-Path $bundle "Contents\x86_64-win\EndlessSmileInspired.vst3"
if (!(Test-Path $binary -PathType Leaf)) { throw "Build returned success but no VST3 binary: $binary" }

$python = Get-Command python -ErrorAction SilentlyContinue
if (!$python) { throw "Python 3 is required to validate and stage the preview package" }
$commit = "local-working-tree"
$branch = "local"
try {
    $gitCommit = (git -C $PSScriptRoot rev-parse HEAD 2>$null).Trim()
    if ($LASTEXITCODE -eq 0 -and $gitCommit) { $commit = $gitCommit }
    $gitBranch = (git -C $PSScriptRoot branch --show-current 2>$null).Trim()
    if ($LASTEXITCODE -eq 0 -and $gitBranch) { $branch = $gitBranch }
} catch {
    Write-Warning "Git provenance unavailable; package will be marked local-working-tree"
}

$previewDir = Join-Path $buildDir "preview-artifacts"
if (Test-Path $previewDir) { Remove-Item -LiteralPath $previewDir -Recurse -Force }
python scripts/prepare_vst3_preview.py `
    --bundle $bundle `
    --vst3-root $vst3Root `
    --output-dir $previewDir `
    --source-commit $commit `
    --workflow-run-id "local" `
    --branch $branch
if ($LASTEXITCODE -ne 0) { throw "VST3 preview package verification failed" }

Write-Host "Verified Windows x64 VST3 module: $binary"
Write-Host "Staged flat preview package: $previewDir"
