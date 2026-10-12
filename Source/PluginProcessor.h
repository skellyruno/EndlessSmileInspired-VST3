#pragma once
#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
class EndlessSmileInspiredAudioProcessor : public juce::AudioProcessor
{
public:
    EndlessSmileInspiredAudioProcessor();
    void prepareToPlay(double,int) override; void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;
    juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override{return true;}
    const juce::String getName() const override; bool acceptsMidi() const override; bool producesMidi() const override;
    bool isMidiEffect() const override; double getTailLengthSeconds() const override;
    int getNumPrograms() override; int getCurrentProgram() override; void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override; void changeProgramName(int,const juce::String&) override;
    void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
    juce::AudioProcessorValueTreeState state;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
private:
    std::atomic<float>* intensity=nullptr; std::atomic<float>* preset=nullptr; std::atomic<float>* mix=nullptr;
    double sampleRate=44100.0;
    juce::dsp::Reverb reverb;
    std::array<juce::dsp::StateVariableTPTFilter<float>,2> hp;
    std::array<juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear>,2> delay;
    juce::SmoothedValue<float> smoothIntensity, smoothMix;
    uint32_t noiseState=0x12345678u;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EndlessSmileInspiredAudioProcessor)
};
