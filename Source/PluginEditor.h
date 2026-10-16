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
    void rebuildBackgroundCache();
    juce::Rectangle<int> getCharacterBounds() const;

    EndlessSmileInspiredAudioProcessor& processor;
    // Declare the LookAndFeel before the controls so it outlives them.
    EndlessSmileInspiredLookAndFeel lookAndFeel;
    juce::Slider intensity;
    juce::ComboBox preset;
    juce::TextButton previousPresetButton;
    juce::TextButton nextPresetButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> intensityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetAttachment;

    // UI-only animation state. Values are smoothed on the message thread so the
    // character feels elastic when the host/automation moves the intensity knob.
    // The static panel (gradient + grain) is rendered once per size, not every frame.
    juce::Image backgroundCache;

    float animationPhase = 0.0f;
    float animatedIntensity = 0.0f;
    float blinkAmount = 0.0f;
    int animationFrame = 0;
    int framesUntilBlink = 82;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EndlessSmileInspiredAudioProcessorEditor)
};
