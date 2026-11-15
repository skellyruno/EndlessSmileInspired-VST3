#include "PluginProcessor.h"
#include <cstdio>
#include <limits>

int main()
{
    juce::AudioPlayHead transport;
    transport.info.bpm = 120.0;
    transport.info.ppq = 2.0;
    transport.info.playing = true;
    EndlessSmileInspiredAudioProcessor p;
    p.mockPlayHead = &transport;
    p.prepareToPlay (48000, 64);
    juce::AudioBuffer<float> block (2, 64);
    juce::MidiBuffer midi;
    const auto process = [&] { p.processBlock (block, midi); };
    process();
    if (!p.hostBpmValid || !p.hostPlaying || p.hostPpq.load() != 2.0f) return 1;
    transport.info.bpm = std::nullopt;
    transport.info.ppq = std::nullopt;
    transport.info.playing = false;
    process();
    if (p.hostBpmValid || p.hostPlaying || p.hostPpq.load() != -1.0f) return 2;
    transport.info.bpm = std::numeric_limits<double>::quiet_NaN();
    process();
    if (p.hostBpmValid) return 3;
    transport.info.bpm = 0.0;
    process();
    if (p.hostBpmValid) return 4;
    transport.info.bpm = 143.0;
    process();
    if (!p.hostBpmValid || p.hostBpm.load() != 143.0f) return 5;
    p.mockPlayHead = nullptr;
    process();
    if (p.hostBpmValid || p.hostPlaying || p.hostPpq.load() != -1.0f) return 6;
    std::puts ("Transport fallback regression passed");
    return 0;
}
