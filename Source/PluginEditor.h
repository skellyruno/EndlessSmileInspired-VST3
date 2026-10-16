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

    // Position of the category tab along the cassette's preset timeline (0 = first
    // preset, 1 = last). Animated by the editor so the tab slides between presets.
    void setCassetteTabPosition (float newPosition) noexcept { cassetteTabPosition = newPosition; }

    // The dial draws this eased position instead of the raw slider position, so the
    // knob glides smoothly. A negative value means "use the slider's own position".
    void setDialDisplayPosition (float newPosition) noexcept { dialDisplayPosition = newPosition; }

private:
    float cassetteTabPosition = 0.0f;
    float dialDisplayPosition = -1.0f;

    // Everything on the dial that does not depend on the value is drawn once into
    // this image (at physical resolution) and reused, so dragging stays cheap.
    juce::Image dialCache;
    juce::Rectangle<int> dialCacheArea;
    int dialCacheWidth = 0;
    int dialCacheHeight = 0;
    float dialCacheScale = 0.0f;
    float dialCacheStart = 0.0f;
    float dialCacheEnd = 0.0f;
};

class EndlessSmileInspiredAudioProcessorEditor : public juce::AudioProcessorEditor,
                                                 private juce::Timer
{
public:
    explicit EndlessSmileInspiredAudioProcessorEditor (EndlessSmileInspiredAudioProcessor&);
    ~EndlessSmileInspiredAudioProcessorEditor() override;
    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void rebuildBackgroundCache();
    void buildFogTexture();
    juce::Rectangle<int> getFogBottomBand() const;
    juce::Rectangle<int> getFogTopBand() const;
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

    // Tileable fog texture, scrolled horizontally over the whole plugin.
    juce::Image fogTexture;
    float fogOffsetA = 0.0f;
    float fogOffsetB = 0.0f;
    float knobDisplay = 0.0f;

    float tabSlide = 0.0f;
    float animationPhase = 0.0f;
    float animatedIntensity = 0.0f;
    float blinkAmount = 0.0f;
    int animationFrame = 0;
    int framesUntilBlink = 82;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EndlessSmileInspiredAudioProcessorEditor)
};
