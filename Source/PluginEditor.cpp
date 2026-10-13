#include "PluginEditor.h"

EndlessSmileInspiredAudioProcessorEditor::EndlessSmileInspiredAudioProcessorEditor (EndlessSmileInspiredAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (900, 510);
    setResizable (true, true);

    intensity.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    intensity.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 24);
    intensity.setRange (0.0, 110.0, 0.1);
    intensity.setNumDecimalPlacesToDisplay (1);
    intensity.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff24343a));
    intensity.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff3ba8d0));
    intensity.setColour (juce::Slider::thumbColourId, juce::Colour (0xff3ba8d0));
    intensity.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    intensity.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (intensity);

    const juce::StringArray presetNames {
        "SOFT  —  FIST IN THE AIR",
        "SOFT  —  HELIUM JUMPING",
        "MEDIUM  —  NO LIMIT",
        "MEDIUM  —  FACE MELTING JOY",
        "HARD  —  SMACK YOU IN THE FACE",
        "HARD  —  BROKEN TEETH",
        "EXTREME  —  DADA DYNAMITE"
    };
    for (int i = 0; i < presetNames.size(); ++i)
        preset.addItem (presetNames[i], i + 1);
    preset.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1d2b30));
    preset.setColour (juce::ComboBox::textColourId, juce::Colours::white);
    preset.setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff829094));
    addAndMakeVisible (preset);

    mix.setSliderStyle (juce::Slider::LinearHorizontal);
    mix.setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 24);
    mix.setRange (0.0, 1.0, 0.001);
    mix.setNumDecimalPlacesToDisplay (3);
    mix.setColour (juce::Slider::trackColourId, juce::Colour (0xff1d2b30));
    mix.setColour (juce::Slider::thumbColourId, juce::Colour (0xff3ba8d0));
    mix.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    mix.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (mix);

    intensityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.state, "intensity", intensity);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.state, "mix", mix);
    presetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.state, "preset", preset);
    startTimerHz (30);
}

void EndlessSmileInspiredAudioProcessorEditor::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.fillAll (juce::Colour (0xff686667));

    // Soft speckle and inset frame, matching the supplied visual reference.
    juce::Random random (91);
    for (int i = 0; i < 650; ++i)
    {
        const float px = random.nextFloat() * bounds.getWidth();
        const float py = random.nextFloat() * bounds.getHeight();
        g.setColour (juce::Colours::black.withAlpha (random.nextFloat() * 0.035f));
        g.fillEllipse (px, py, 1.0f + random.nextFloat(), 1.0f + random.nextFloat());
    }
    g.setColour (juce::Colour (0xff29272a));
    g.drawRoundedRectangle (bounds.reduced (8.0f), 10.0f, 3.0f);
    g.setColour (juce::Colour (0xffc7c3c2).withAlpha (0.45f));
    g.drawRoundedRectangle (bounds.reduced (13.0f), 8.0f, 1.5f);

    const float t = juce::jlimit (0.0f, 1.0f, (float) intensity.getValue() / 110.0f);
    const float smileX = getWidth() * 0.735f;
    const float smileY = getHeight() * 0.465f;
    const float radius = juce::jmin (getWidth() * 0.155f, getHeight() * 0.275f);

    // Left open arc and the intensity dial arc.
    juce::Path openArc;
    openArc.addCentredArc (getWidth() * 0.22f, getHeight() * 0.245f,
                           getWidth() * 0.042f, getHeight() * 0.165f, 0.0f,
                           juce::MathConstants<float>::pi * 0.62f,
                           juce::MathConstants<float>::pi * 1.38f, true);
    g.setColour (juce::Colour (0xffe2ded9));
    g.strokePath (openArc, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path dialArc;
    dialArc.addCentredArc (getWidth() * 0.50f, getHeight() * 0.285f,
                           getWidth() * 0.108f, getHeight() * 0.19f, 0.0f,
                           juce::MathConstants<float>::pi * 0.16f,
                           juce::MathConstants<float>::pi * (1.82f - 0.25f * t), true);
    g.setColour (juce::Colour (0xff1d2b30));
    g.strokePath (dialArc, juce::PathStrokeType (7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float knobAngle = juce::MathConstants<float>::pi * (0.16f + 1.66f * t);
    const float knobX = getWidth() * 0.50f + std::cos (knobAngle) * getWidth() * 0.108f;
    const float knobY = getHeight() * 0.285f + std::sin (knobAngle) * getHeight() * 0.19f;
    g.setColour (juce::Colour (0xff3ba8d0));
    g.fillEllipse (knobX - 7.5f, knobY - 7.5f, 15.0f, 15.0f);

    // Smiling mascot.
    g.setColour (juce::Colour (0xffff9b2f));
    g.fillEllipse (smileX - radius * 1.03f, smileY - radius * 1.03f, radius * 2.06f, radius * 2.06f);
    g.setColour (juce::Colour (0xffd8f0df));
    g.fillEllipse (smileX - radius * 0.88f, smileY - radius * 0.88f, radius * 1.76f, radius * 1.76f);

    g.setColour (juce::Colour (0xfff04491));
    g.drawLine (smileX - radius * 0.80f, smileY - radius * 0.34f,
                smileX - radius * (1.10f + 0.08f * t), smileY - radius * 1.05f, 7.0f);
    g.drawLine (smileX + radius * 0.80f, smileY - radius * 0.34f,
                smileX + radius * (1.10f + 0.08f * t), smileY - radius * 1.05f, 7.0f);
    g.setColour (juce::Colour (0xffa42f59));
    const float eyeHeight = radius * (0.29f + 0.08f * t);
    g.fillEllipse (smileX - radius * 0.48f, smileY - radius * 0.30f, radius * 0.20f, eyeHeight);
    g.fillEllipse (smileX + radius * 0.28f, smileY - radius * 0.30f, radius * 0.20f, eyeHeight);

    const float mouthWidth = radius * (0.57f + 0.42f * t);
    const float mouthDrop = radius * (0.10f + 0.25f * t);
    juce::Path smile;
    smile.startNewSubPath (smileX - mouthWidth, smileY + radius * 0.20f);
    smile.quadraticTo (smileX, smileY + radius * 0.20f + mouthDrop * 2.0f,
                       smileX + mouthWidth, smileY + radius * 0.20f);
    g.setColour (juce::Colour (0xfff04491));
    g.strokePath (smile, juce::PathStrokeType (radius * 0.075f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    juce::Path mouthShadow;
    mouthShadow.startNewSubPath (smileX - mouthWidth, smileY + radius * 0.22f);
    mouthShadow.quadraticTo (smileX, smileY + radius * 0.22f + mouthDrop * 2.0f,
                             smileX + mouthWidth, smileY + radius * 0.22f);
    g.setColour (juce::Colour (0xff802747));
    g.strokePath (mouthShadow, juce::PathStrokeType (radius * 0.035f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour (juce::Colours::black);
    g.setFont (juce::Font (juce::FontOptions (16.0f, juce::Font::bold)));
    g.drawText ("INTENSITY", getWidth() * 0.405, getHeight() * 0.48f,
                getWidth() * 0.18f, 22, juce::Justification::centred);
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions (15.0f)));
    g.drawText (juce::String (intensity.getValue(), 2), getWidth() * 0.405, getHeight() * 0.505f,
                getWidth() * 0.18f, 22, juce::Justification::centred);

    g.setColour (juce::Colour (0xffd5d0c9).withAlpha (0.82f));
    g.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
    g.drawText ("SKELLY AUDIO   •   BUILD-UP / TRANSITION EFFECT", 20, getHeight() - 24,
                getWidth() - 40, 14, juce::Justification::centredLeft);
}

void EndlessSmileInspiredAudioProcessorEditor::resized()
{
    const auto bounds = getLocalBounds();
    const int dialSize = juce::jmin (190, juce::roundToInt (bounds.getHeight() * 0.35f));
    intensity.setBounds (bounds.getWidth() / 2 - dialSize / 2, 20, dialSize, dialSize + 34);
    preset.setBounds (36, bounds.getHeight() - 122, juce::roundToInt (bounds.getWidth() * 0.48f), 40);
    mix.setBounds (36, bounds.getHeight() - 78, juce::roundToInt (bounds.getWidth() * 0.48f), 32);
}

void EndlessSmileInspiredAudioProcessorEditor::timerCallback()
{
    repaint();
}
