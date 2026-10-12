# Endless Smile Inspired — JUCE VST3 starter

A clean-room, original implementation inspired by the one-knob build-up workflow. It is not affiliated with or endorsed by Dada Life, and does not contain their proprietary code or character artwork.

## Features
- JUCE VST3 + Standalone CMake project
- Intensity parameter from 0 to 110%, preset selector (7 modes), and Mix
- High-pass filtering, reverb, delay, saturation, noise modulation and a rising resonant component
- Intensity smoothing and host-automatable parameters
- Animated original cartoon-style face that reacts to intensity

## Build on Windows
1. Install Visual Studio 2022 with **Desktop development with C++**, CMake, and Git.
2. Open **Developer PowerShell for VS 2022** in this folder.
3. Run `.\build-windows.ps1`.
4. The VST3 and Standalone builds will be placed in `build\EndlessSmileInspired_artefacts\Release\`.

First build requires internet to fetch JUCE 8.0.4.

## Notes / next engineering pass
This is a functional prototype, not a bit-for-bit recreation of the commercial plugin. The seven preset curves are currently starter voicings and need listening-based tuning. The audio thread uses preallocated buffers; run DAW stress tests and sanitizer/host validation before production use.
