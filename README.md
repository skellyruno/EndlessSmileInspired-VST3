# Skeleton Graveyard — JUCE VST3 build-up / transition effect

A one-knob build-up and transition effect with a skeleton mascot, in an original
graveyard-themed interface. It is an independent implementation inspired by the
one-knob build-up workflow. It is not affiliated with or endorsed by Dada Life or
Tailored Noise and contains none of their code or artwork.

## Controls

- **Intensity** (0–110): the single macro. The mascot follows it instantly, from
  slumped and miserable at zero to raised arms and a wide grin at the top.
- **Preset** (16): chosen from the dropdown, the arrows, or the host. A category tab
  slides along a timeline as you move through presets.
- **Mix** (host-automatable parameter, default 100%): no longer shown in the window.
- **Corruption** (0-1): a one-slider macro. It adds to the effective Intensity (so it works
  even with the knob at zero), pushes the saturator, reverb and a little hiss, and infects
  the skeleton: bones turn toxic and veins creep up the skull as the slider rises.
- **SYNC**: when on and the host reports a tempo, the delay time and the Hard-preset riser
  (2 bars) follow it, and the bats flap in time with the beat. Without a host tempo the
  fixed times are used.
- **LITE**: a pencil-ink background with no gradients, glow or fog; the lightest mode.
- **SKIN**: cycles TOXIC, BLOOD, ICE and INK (monochrome). LITE and SKIN are saved with the
  project.
- Each preset category is a different scene: SOFT dusk, MEDIUM night, HARD blood-red night,
  EXTREME all-dead, GRAVEYARD toxic green.
- The skeleton reacts to the input: sudden loud transients widen its eyes.
- Hovering over a control or a preset shows a one-line eerie story.

## Presets

| # | Category | Preset | What it does |
|---|----------|--------|--------------|
| 1 | SOFT | Fist in the Air | Reverb, high-pass sweep to about 200 Hz |
| 2 | SOFT | Helium Jumping | Reverb, high-pass to about 200 Hz, delay |
| 3 | MEDIUM | No Limit | Reverb, full high-pass sweep |
| 4 | MEDIUM | Face Melting Joy | Reverb, full high-pass sweep, delay |
| 5 | HARD | Smack You in the Face | Reverb, full sweep, noise, continual looping riser |
| 6 | HARD | Broken Teeth | As above, plus delay |
| 7 | EXTREME | Dada Dynamite | All effects; noise and riser driven directly by Intensity |
| 8 | GRAVEYARD | Midnight Howl | Wind through a drifting resonant band |
| 9 | GRAVEYARD | Banshee Scream | Rising, vibrato-laden shriek with echo |
| 10 | GRAVEYARD | Ghost Whisper | Breathy whispers with a long fading echo |
| 11 | GRAVEYARD | Rattling Bones | Random dry clicks, denser as Intensity rises |
| 12 | GRAVEYARD | Crypt Drone | Low detuned drone through a moving vowel |
| 13 | GRAVEYARD | Wailing Wraith | Wavering, mournful wail with a trailing echo |
| 14 | GRAVEYARD | Zombie Groan | Low growling groan that swells |
| 15 | GRAVEYARD | Coffin Creak | Long resonant creak wandering in pitch |
| 16 | GRAVEYARD | Haunted Bells | Slow inharmonic bell strikes, faster with Intensity |

At Intensity 0 the output is exactly the dry signal in every preset. The first seven
preset positions are unchanged from earlier versions, so older sessions still load.

## Signal path (per sample)

high-pass (sweeps with Intensity) → wet-only reverb, with the high-passed dry added
back at unity gain → optional delay (with feedback in Ghost Whisper and Wailing Wraith)
→ level-compensated saturator → noise/riser (Hard, Extreme) or synthesised graveyard
voice → soft ceiling at 1.0 → crossfade with the dry signal by Intensity, then Mix.

Tunable constants: `hardRiserLoopSeconds` (top of `PluginProcessor.cpp`, 8 s; the riser length
when SYNC is off or the host reports no tempo) and the per-voice `level` values inside each graveyard case.

## Artwork

The main dial follows the Skell Necromancer project's REANIMATE dial: a dark well
(`Assets/dial_well.png`), 90 ticks, a violet-green-cyan value arc, a bone pointer and a
brushed-metal cap (`Assets/knob_cap_big.png`) that turns with the value. Both images are embedded
with `juce_add_binary_data` (CMake target `EndlessSmileAssets`). As the value rises the dial powers up: a green halo, glowing arc and ticks, a rim light on the
cap and a lit pointer tip (LITE mode turns this off). The well is cached at physical
resolution; the cap is rescaled once and only rotated while you drag. The Necromancer's
animated rune ring is not included.

## Build on Windows

1. Install Visual Studio 2022 with **Desktop development with C++**, CMake, Git and Python 3.
2. Open **Developer PowerShell for VS 2022** in this folder and run `.\build-windows.ps1`,
   or run the **Build and test Skeleton Graveyard** GitHub Actions workflow.
3. The VST3 bundle is built at `build\EndlessSmileInspired_artefacts\Release\VST3\EndlessSmileInspired.vst3`.
4. The script also validates and stages a flat preview package in `build\preview-artifacts\`
   with `release-manifest.json` and `SHA256SUMS.txt`.

The first build needs internet to fetch JUCE 8.0.4. The build target, VST3 filename and
host identity remain `EndlessSmileInspired`; the display name is "Skeleton Graveyard".
Do not rename the VST3 bundle or change its stable identity fields without a compatibility
migration plan.

## Offline DSP tests

`tests/run.sh` compiles the real `PluginProcessor.cpp` against a small stand-in for the JUCE
classes it uses (`tests/mock`; the reverb is a port of JUCE's Freeverb) and runs
`tests/dsp_test.cpp`: bit-exact dry at Intensity 0, no NaN/inf and no output above 1.0 for every
preset (22.05-192 kHz, block sizes 1-4096, mono/stereo, hot input, tempo sync, automation),
Mix 0 = dry, recovery after a NaN/inf sample, very hot input, graveyard voices
balanced on silent input (-34..-22 dBFS), reverb tail decay, zero latency and CPU. `tests/ui_check.sh` type-checks `PluginEditor.cpp` against a signature-level JUCE GUI stand-in
(`tests/mock_ui`; it does not render). Both need only g++. It does not replace listening or
pluginval.

## Verification status — please read

- Main commit `059a8151e64be4c7223667eef81780792da8526a` compiled the plugin against real
  JUCE 8.0.4 on Windows in Actions run #25 on November 15, 2026. The Linux offline DSP test
  and editor API signature check also passed in that run.
- The run produced a real Windows x64 VST3 bundle. Release engineering inspected the
  downloaded artifact and confirmed the inner module is AMD64 PE32+ and the three existing
  VST3 class IDs match the protected identity.
- That inspection also found the Actions download was a ZIP containing a second ZIP, and
  JUCE's generated `moduleinfo.json` contained trailing commas. The release branch adds a
  strict package normalizer/verifier, SHA-256 checksums and source/run provenance to prevent
  those packaging defects from recurring.
- No pluginval run, FL Studio scan/load, project recall, real audio A/B, rendered UI capture,
  or owner-machine CPU/latency acceptance has been completed. Those remain release gates.

## Known behaviour worth knowing

- In the full-sweep presets (3-7) the dry signal is high-passed up to 15 kHz, but the reverb is
  fed through its own high-pass capped at 2.5 kHz (`washMaxCutoff`), so a reverb wash remains at
  the top instead of silence.
- The fog drift repaints less often on slow machines (it adapts to measured repaint
  time) and pauses while you drag the knob or the preset tab slides.

## v1.2.0 upgrade and CI notes

The VST3 product name, bundle ID, manufacturer and plugin codes are deliberately
unchanged to preserve host plugin identification. The first three automation
parameter IDs (`intensity`, `preset`, `mix`) are unchanged, the original seven
preset slots retain their indices, and `sync`/`corruption` are added at the end.
Run `python3 tests/check_compatibility.py` for a static compatibility guard.
The GitHub Actions workflow runs mock DSP and editor signature checks on Linux
plus a real JUCE VST3 compile on Windows for PRs and manual requests.
These checks do not establish FL Studio project recall, pluginval, UI rendering,
real-world audio quality or real-host CPU/latency.

- The first PR Windows build passed, but its compiler log exposed MSVC warning C4458 because a local `playHead` name shadowed JUCE's `AudioProcessor::playHead`. The follow-up renames that local to `hostPlayHead` and guards against regression.
- PR artifacts now record the actual PR head SHA and branch rather than GitHub's synthetic `1/merge` ref. The workflow also uses Node 24-compatible official Actions releases.

## Windows preview artifact contract

The Actions workflow uploads the staged directory directly; GitHub creates the one download
archive. Do not put a pre-compressed ZIP inside the uploaded artifact. The archive contains
exactly one `EndlessSmileInspired.vst3` bundle plus `release-manifest.json` and
`SHA256SUMS.txt`.

The package step verifies the VST3 bundle layout, AMD64 PE32+ machine type, single module
binary, vendor/version metadata and the three protected class IDs. It normalizes only the
staged copy of JUCE's `moduleinfo.json` to strict JSON; source build output and host IDs are
not rewritten. The manifest records the PR head/source commit, branch, workflow run, file sizes and
SHA-256 hashes. Five regression tests cover normalization, flat staging, wrong architecture,
duplicate bundles and duplicate class IDs.

The Actions artifact is retained for seven days. This repository is public, so its Actions
artifact is not a private customer download. It is explicitly preview-only and is not a
commercial release. The current vendor URL and support email are blank in the plugin metadata;
the manifest records those omissions rather than inventing contact details. Licensing and
attribution, FL Studio acceptance, real audio A/B, CPU/latency, pricing/trial and distribution
approval remain open launch gates.
