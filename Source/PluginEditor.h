#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class EndlessSmileInspiredAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit EndlessSmileInspiredAudioProcessorEditor(EndlessSmileInspiredAudioProcessor&);
    void paint(juce::Graphics&) override; void resized() override;
private:
    void timerCallback() override;
    EndlessSmileInspiredAudioProcessor& processor;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> intensityAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetAttach;
    juce::Slider intensity,mix; juce::ComboBox preset;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EndlessSmileInspiredAudioProcessorEditor)
};
