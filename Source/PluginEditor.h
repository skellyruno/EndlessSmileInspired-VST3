#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Custom JUCE look-and-feel keeps the rotary dial consistent across Windows hosts.
class EndlessSmileInspiredLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool isMouseOverButton,
                         bool isButtonDown) override;
};

class EndlessSmileInspiredAudioProcessorEditor : public juce::AudioProcessorEditor,
                                                 private juce::Timer
{
public:
    explicit EndlessSmileInspiredAudioProcessorEditor (EndlessSmileInspiredAudioProcessor&);
    ~EndlessSmileInspiredAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    EndlessSmileInspiredAudioProcessor& processor;
    // Declare the LookAndFeel before the controls so it outlives them.
    EndlessSmileInspiredLookAndFeel lookAndFeel;
    juce::Slider intensity;
    juce::Slider mix;
    juce::ComboBox preset;
    juce::TextButton previousPresetButton;
    juce::TextButton nextPresetButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> intensityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetAttachment;

    // UI-only motion clock; animation is intensity weighted and audio-thread independent.
    float animationPhase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EndlessSmileInspiredAudioProcessorEditor)
};
