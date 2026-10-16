#pragma once
#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

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

private:
    std::atomic<float>* intensityParameter = nullptr;
    std::atomic<float>* presetParameter = nullptr;
    std::atomic<float>* mixParameter = nullptr;

    double currentSampleRate = 44100.0;
    juce::Reverb reverb;
    std::array<juce::dsp::StateVariableTPTFilter<float>, 2> highPassFilters;
    std::array<juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>, 2> delayLines;
    juce::SmoothedValue<float> smoothedIntensity;
    juce::SmoothedValue<float> smoothedMix;
    juce::SmoothedValue<float> smoothedDelaySeconds;
    uint32_t noiseState = 0x12345678u;
    uint32_t noiseStateRight = 0x9e3779b9u; // independent stream for the right channel
    float riserPhase = 0.0f;
    float riserRamp = 0.0f;                 // 0..1 position in the Hard-preset riser loop

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EndlessSmileInspiredAudioProcessor)
};
