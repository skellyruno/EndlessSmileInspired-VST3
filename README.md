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

Tunable constants: `hardRiserLoopSeconds` (top of `PluginProcessor.cpp`, 8 s placeholder
for the Hard-preset riser) and the per-voice `level` values inside each graveyard case.

## Artwork

The knob face is `Assets/KnobEmblem.png` (a 512 px circular crop of the neon skull-and-gear
emblem supplied for this project), embedded into the plugin at build time by
`juce_add_binary_data` in `CMakeLists.txt`. If the image fails to decode, the dial falls
back to the drawn metal knob. Only ship builds containing artwork you have the rights to.

## Build on Windows

1. Install Visual Studio 2022 with **Desktop development with C++**, CMake and Git.
2. Open **Developer PowerShell for VS 2022** in this folder and run `.\build-windows.ps1`,
   or run the **Build plugin artifacts** GitHub Actions workflow.
3. Output: `build\EndlessSmileInspired_artefacts\Release\`.

The first build needs internet to fetch JUCE 8.0.4. The build target and file name
are still `EndlessSmileInspired`; only the window title and the name reported to the
host say "Skeleton Graveyard". Renaming the files means changing `PRODUCT_NAME` in
`CMakeLists.txt` together with the paths in `.github/workflows/build.yml`.

## Verification status — please read

- The interface and audio code up to the fog/knob/preset-timeline work was built by
  CI and checked from screenshots.
- The latest changes (performance restructuring, the four newest presets, tooltips,
  dropdown headings) have only been type-checked against a minimal stand-in for the
  JUCE API with g++, not built against real JUCE.
- Nothing has been measured on a real host. Loudness figures come from a numeric model
  of the signal path that leaves out JUCE's reverb. Graveyard voice levels were set
  from numeric simulations and need listening checks.
- Not yet done: pluginval, DAW session recall/automation tests, tests at 44.1/48/96 kHz
  and different block sizes, and CPU profiling on a real machine.

## Known behaviour worth knowing

- Presets 3 and 4 (full high-pass sweep) are nearly silent at maximum Intensity, because
  the sweep reaches the top of the spectrum. That is the sweep working as described.
- The fog drift repaints less often on slow machines (it adapts to measured repaint
  time) and pauses while you drag the knob or the preset tab slides.
