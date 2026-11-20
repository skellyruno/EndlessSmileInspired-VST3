# Skeleton Graveyard 1.2.0 — release verification log

## Protected baseline

- Main commit: `059a8151e64be4c7223667eef81780792da8526a`
- Successful Windows build: [Actions run #43769291311](https://github.com/skellyruno/EndlessSmileInspired-VST3/actions/runs/43769291311)
- Baseline artifact: [#14183556829](https://github.com/skellyruno/EndlessSmileInspired-VST3/actions/runs/43769291311/artifacts/14183556829)
- Baseline archive SHA-256: `06b13c55fc6859ecbc4a54a9d3672c9194497120c7653c4c0156ee82089557b9`
- Baseline VST3 module SHA-256: `5a6061d16c9f5bf7b3f943320fa472ac0d47b8b8b6495743abbf2077c425c37b`
- Baseline moduleinfo SHA-256: `724fad59415df4e177de12132836b7b76728cc5fcc4b99243251febc77bb1fe2`

The baseline archive was downloaded and inspected. It contained an outer GitHub artifact ZIP with a second ZIP inside. The inner bundle had one Windows x64 VST3 module and JUCE-generated trailing commas in `Contents/Resources/moduleinfo.json`.

## PR validation run 1

- Run: [#44667555195](https://github.com/skellyruno/EndlessSmileInspired-VST3/actions/runs/44667555195)
- Tested PR head: `f1d963119db10d9564b92d9e5aed82729a95dccd`
- Result: Linux offline job **PASS**; Windows JUCE 8.0.4 / MSVC x64 build **PASS**; package verification **PASS**; upload **PASS**.
- Artifact: [#14599295650](https://github.com/skellyruno/EndlessSmileInspired-VST3/actions/runs/44667555195/artifacts/14599295650)
- Artifact SHA-256: `64612f671ad5851e4375c7643692a15bddb5d703cefb72e4b540b1d082b4437d`
- Artifact expires: 2026-11-27 03:27:56 UTC (seven-day retention).

The downloaded artifact was independently inspected: four ZIP entries, exactly one `EndlessSmileInspired.vst3` bundle, no nested ZIP, one module binary, strict-JSON module metadata, and matching SHA-256 entries in the manifest/checksum file. The module is AMD64 PE32+ (`0x8664`, optional-header `0x020B`), size 6,414,848 bytes. Its SHA-256 is `e9b41e16eb40b5a79847e07ab71b06c12f3a7bd6e946f0e0eb54e11ed0407bb2`.

Two issues were found during post-build review:

1. For a `pull_request` event, `github.sha` and `github.ref_name` recorded GitHub's synthetic `1/merge` ref rather than the actual PR head SHA/branch. The follow-up workflow commit uses `github.event.pull_request.head.sha || github.sha` and `github.head_ref || github.ref_name`.
2. MSVC emitted warning C4458 at `Source/PluginProcessor.cpp:163`: local `playHead` shadowed JUCE's `AudioProcessor::playHead`. The follow-up renames it to `hostPlayHead` and adds a compatibility regression assertion.

The new PR build's module SHA differs from the preserved baseline module SHA despite equal file size. The PR changes no DSP or UI code, but bit-for-bit reproducibility has not been established; no sound-change conclusion is drawn from that hash difference.

## Follow-up revalidation

- Follow-up source commit: `03ed989efd9b4b0d1b4329538e486eadd23450e2`
- Changes: correct PR-head provenance; rename the shadowing local; add a warning-regression assertion; upgrade Actions to Node 24-compatible official releases.
- Status at log creation: **CI revalidation pending**. Do not promote artifact #14599295650 as the final PR artifact; its source provenance is superseded.

## Release gates still open

- FL Studio scan/load, routing, project recall and automation
- pluginval
- Real audio A/B, DSP listening, target-machine CPU/latency
- Fresh rendered UI capture and interaction inspection
- JUCE licensing and third-party attribution approval
- Vendor URL and support email
- Storefront/email signup, updater/rollback, pricing/trial and commercial distribution approval

All artifacts remain preview-only. No release tag, public release, website deployment or merge has been made.
