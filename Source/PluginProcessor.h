#pragma once
#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <cmath>

// Resonant band-pass (TPT state-variable form) with unity gain at the centre frequency, so each
// graveyard voice's loudness is set by its own gain constant and never depends on the resonance.
class ResonantBandPass
{
public:
    void prepare (double newSampleRate) noexcept { sampleRate = (float) newSampleRate; reset(); update(); }
    void reset() noexcept { s1 = s2 = 0.0f; }
    void setResonance (float q) noexcept { resonance = q; update(); }
    void setCutoffFrequency (float hz) noexcept { cutoff = hz; update(); }

    float process (float x) noexcept
    {
        const float highPass = h * (x - s1 * (g + r2) - s2);
        const float bandPass = highPass * g + s1;
        s1 = highPass * g + bandPass;
        const float lowPass = bandPass * g + s2;
        s2 = bandPass * g + lowPass;
        return bandPass * r2;
    }

private:
    void update() noexcept
    {
        const float fc = std::fmin (std::fmax (cutoff, 20.0f), 0.49f * sampleRate);
        g = std::tan (juce::MathConstants<float>::pi * fc / sampleRate);
        r2 = 1.0f / resonance;
        h = 1.0f / (1.0f + r2 * g + g * g);
    }

    float sampleRate = 44100.0f, cutoff = 1000.0f, resonance = 1.0f;
    float g = 0.0f, r2 = 1.0f, h = 1.0f, s1 = 0.0f, s2 = 0.0f;
};

class EndlessSmileInspiredAudioProcessor : public juce::AudioProcessor
{
public:
    EndlessSmileInspiredAudioProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override;
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState state;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    // Read by the editor (message thread), written by processBlock (audio thread).
    std::atomic<float> hostBpm { 120.0f };       // last tempo reported by the host
    std::atomic<bool> hostBpmValid { false };    // true once a host has reported a tempo
    std::atomic<bool> hostPlaying { false };
    std::atomic<float> hostPpq { -1.0f };        // position in quarter notes, or -1 if unknown
    std::atomic<float> inputLevel { 0.0f };      // peak envelope of the incoming signal, for the mascot

private:
    std::atomic<float>* intensityParameter = nullptr;
    std::atomic<float>* presetParameter = nullptr;
    std::atomic<float>* mixParameter = nullptr;
    std::atomic<float>* syncParameter = nullptr;
    std::atomic<float>* corruptionParameter = nullptr;
    float levelEnv = 0.0f;

    double currentSampleRate = 44100.0;
    juce::Reverb reverb;
    std::array<juce::dsp::StateVariableTPTFilter<float>, 2> highPassFilters;
    std::array<juce::dsp::StateVariableTPTFilter<float>, 2> washFilters;   // high-pass feeding the reverb (capped lower)
    std::array<juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>, 2> delayLines;
    juce::SmoothedValue<float> smoothedIntensity;
    juce::SmoothedValue<float> smoothedMix;
    juce::SmoothedValue<float> smoothedDelaySeconds;
    juce::SmoothedValue<float> smoothedCorruption;
    uint32_t noiseState = 0x12345678u;
    uint32_t noiseStateRight = 0x9e3779b9u; // independent stream for the right channel
    float riserPhase = 0.0f;
    float riserRamp = 0.0f;                 // 0..1 position in the Hard-preset riser loop

    // Graveyard presets: resonant band-pass voices plus slow LFOs and oscillator phases.
    std::array<ResonantBandPass, 2> scaryBandFilters;
    float lfoPhaseA = 0.0f, lfoPhaseB = 0.0f, lfoPhaseC = 0.0f, lfoPhaseD = 0.0f;
    float vibratoPhase = 0.0f, scarePhase = 0.0f, scarePhase2 = 0.0f;
    float roughPhase = 0.0f;                // fast amplitude wobble (growl, creak)
    float bellPhase[4] { 0.0f, 0.0f, 0.0f, 0.0f };
    float bellEnv[4] { 0.0f, 0.0f, 0.0f, 0.0f };
    float bellFreq = 196.0f;
    float clickEnv[2] { 0.0f, 0.0f };       // rattling-bones click envelopes
    float feedbackDamp[2] { 0.0f, 0.0f };   // low-pass inside the ghost echo's feedback path

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EndlessSmileInspiredAudioProcessor)
};
