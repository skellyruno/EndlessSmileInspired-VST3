#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class EndlessSmileInspiredAudioProcessorEditor : public juce::AudioProcessorEditor,
                                                 private juce::Timer
{
public:
    explicit EndlessSmileInspiredAudioProcessorEditor (EndlessSmileInspiredAudioProcessor&);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    EndlessSmileInspiredAudioProcessor& processor;
    juce::Slider intensity;
    juce::Slider mix;
    juce::ComboBox preset;
    juce::Label intensityLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> intensityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EndlessSmileInspiredAudioProcessorEditor)
};
