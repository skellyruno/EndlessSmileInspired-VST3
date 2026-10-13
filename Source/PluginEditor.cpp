#include "PluginEditor.h"
#include <cmath>

namespace
{
    const juce::Colour panelGrey  (0xff686667);
    const juce::Colour frameDark  (0xff29272a);
    const juce::Colour ink        (0xff1d2b30);
    const juce::Colour cyan       (0xff39afd2);
    const juce::Colour pink       (0xfff04491);
    const juce::Colour pinkShadow (0xff812746);
    const juce::Colour orange     (0xffff9b2f);
    const juce::Colour faceMint   (0xffd8f0df);

    void strokeRoundedPath (juce::Graphics& g, const juce::Path& path,
                            juce::Colour colour, float thickness)
    {
        g.setColour (colour);
        g.strokePath (path, juce::PathStrokeType (thickness,
                        juce::PathStrokeType::curved,
                        juce::PathStrokeType::rounded));
    }
}

void EndlessSmileInspiredLookAndFeel::drawRotarySlider (juce::Graphics& g,
    int x, int y, int width, int height, float sliderPosProportional,
    float rotaryStartAngle, float rotaryEndAngle, juce::Slider&)
{
    juce::ignoreUnused (rotaryStartAngle, rotaryEndAngle);

    const float side = (float) juce::jmin (width, height);
    const float cx = x + width * 0.5f;
    const float cy = y + height * 0.47f;
    const float radius = side * 0.455f;
    const float start = juce::MathConstants<float>::pi * 0.70f;
    const float end = juce::MathConstants<float>::pi * 2.30f;
    const float clamped = juce::jlimit (0.0f, 1.0f, sliderPosProportional);
    const float current = start + (end - start) * clamped;

    // One open track and one value arc; no overlapping native JUCE rotary artwork.
    juce::Path track;
    track.addCentredArc (cx, cy, radius, radius, 0.0f, start, end, true);
    strokeRoundedPath (g, track, juce::Colour (0xff1d2b30), 7.0f);

    if (clamped > 0.0001f)
    {
        juce::Path active;
        active.addCentredArc (cx, cy, radius, radius, 0.0f, start, current, true);
        strokeRoundedPath (g, active, cyan, 7.0f);
    }

    const float markerX = cx + std::cos (current) * radius;
    const float markerY = cy + std::sin (current) * radius;
    g.setColour (cyan);
    g.fillEllipse (markerX - 6.5f, markerY - 6.5f, 13.0f, 13.0f);

    // Dark, softly metallic knob body.
    const float knobR = radius * 0.53f;
    g.setColour (juce::Colour (0xffb9b7b8).withAlpha (0.75f));
    g.fillEllipse (cx - knobR - 4.0f, cy - knobR - 4.0f,
                   (knobR + 4.0f) * 2.0f, (knobR + 4.0f) * 2.0f);

    juce::ColourGradient body (juce::Colour (0xff555557), cx - knobR * 0.65f, cy - knobR,
                               juce::Colour (0xff111315), cx + knobR, cy + knobR, true);
    g.setGradientFill (body);
    g.fillEllipse (cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f);

    g.setColour (juce::Colour (0xffe7e5e5).withAlpha (0.60f));
    g.drawEllipse (cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f, 1.4f);

    // Restrained diagonal highlight suggests a polished physical control.
    juce::Path highlight;
    highlight.startNewSubPath (cx - knobR * 0.62f, cy - knobR * 0.40f);
    highlight.lineTo (cx + knobR * 0.43f, cy + knobR * 0.48f);
    g.setColour (juce::Colours::white.withAlpha (0.20f));
    g.strokePath (highlight, juce::PathStrokeType (2.2f,
                    juce::PathStrokeType::curved,
                    juce::PathStrokeType::rounded));

    const float pointerAngle = juce::MathConstants<float>::pi * (0.72f + 1.08f * clamped);
    g.setColour (juce::Colour (0xffe4e0df).withAlpha (0.8f));
    g.drawLine (cx, cy, cx + std::cos (pointerAngle) * knobR * 0.78f,
                cy + std::sin (pointerAngle) * knobR * 0.78f, 2.4f);
}

void EndlessSmileInspiredLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height,
    bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box)
{
    juce::ignoreUnused (isButtonDown, buttonX, buttonY, buttonW, buttonH, box);
    const auto r = juce::Rectangle<float> (0.5f, 0.5f, (float) width - 1.0f, (float) height - 1.0f);
    g.setColour (juce::Colour (0xff1d2b30));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (juce::Colour (0xff829094));
    g.drawRoundedRectangle (r, 4.0f, 1.0f);

    const float arrowCX = width - 24.0f;
    const float arrowCY = height * 0.5f;
    juce::Path arrow;
    arrow.startNewSubPath (arrowCX - 7.0f, arrowCY - 2.0f);
    arrow.lineTo (arrowCX, arrowCY + 4.0f);
    arrow.lineTo (arrowCX + 7.0f, arrowCY - 2.0f);
    strokeRoundedPath (g, arrow, juce::Colour (0xffe5e8e8), 2.0f);
}

EndlessSmileInspiredAudioProcessorEditor::EndlessSmileInspiredAudioProcessorEditor
    (EndlessSmileInspiredAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (900, 510);
    setResizable (true, true);
    setResizeLimits (680, 390, 1400, 790);
    setLookAndFeel (&lookAndFeel);

    intensity.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    intensity.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    intensity.setRange (0.0, 110.0, 0.1);
    intensity.setNumDecimalPlacesToDisplay (2);
    intensity.setRotaryParameters (juce::MathConstants<float>::pi * 0.70f,
                                  juce::MathConstants<float>::pi * 2.30f, true);
    intensity.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    intensity.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (intensity);

    // ASCII separators avoid mojibake in MSVC builds with inconsistent source encodings.
    const juce::StringArray presetNames {
        "SOFT  |  FIST IN THE AIR",
        "SOFT  |  HELIUM JUMPING",
        "MEDIUM  |  NO LIMIT",
        "MEDIUM  |  FACE MELTING JOY",
        "HARD  |  SMACK YOU IN THE FACE",
        "HARD  |  BROKEN TEETH",
        "EXTREME  |  DADA DYNAMITE"
    };
    for (int i = 0; i < presetNames.size(); ++i)
        preset.addItem (presetNames[i], i + 1);
    preset.setJustificationType (juce::Justification::centredLeft);
    preset.setColour (juce::ComboBox::backgroundColourId, ink);
    preset.setColour (juce::ComboBox::textColourId, juce::Colours::white);
    preset.setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff829094));
    preset.setColour (juce::ComboBox::arrowColourId, juce::Colours::white);
    addAndMakeVisible (preset);

    mix.setSliderStyle (juce::Slider::LinearHorizontal);
    mix.setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 24);
    mix.setRange (0.0, 1.0, 0.001);
    mix.setNumDecimalPlacesToDisplay (3);
    mix.setColour (juce::Slider::trackColourId, ink);
    mix.setColour (juce::Slider::thumbColourId, cyan);
    mix.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    mix.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    mix.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (mix);

    intensityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (p.state, "intensity", intensity);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (p.state, "mix", mix);
    presetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        (p.state, "preset", preset);

    startTimerHz (30);
}

EndlessSmileInspiredAudioProcessorEditor::~EndlessSmileInspiredAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void EndlessSmileInspiredAudioProcessorEditor::paint (juce::Graphics& g)
{
    const float w = (float) getWidth();
    const float h = (float) getHeight();
    const auto bounds = getLocalBounds().toFloat();
    g.fillAll (panelGrey);

    // Fine, deterministic speckle and double inset frame.
    juce::Random random (91);
    for (int i = 0; i < 500; ++i)
    {
        const float px = random.nextFloat() * w;
        const float py = random.nextFloat() * h;
        g.setColour (juce::Colours::black.withAlpha (random.nextFloat() * 0.025f));
        g.fillEllipse (px, py, 1.0f, 1.0f);
    }
    g.setColour (frameDark);
    g.drawRoundedRectangle (bounds.reduced (8.0f), 10.0f, 3.0f);
    g.setColour (juce::Colour (0xffc7c3c2).withAlpha (0.55f));
    g.drawRoundedRectangle (bounds.reduced (13.0f), 8.0f, 1.4f);

    const float t = juce::jlimit (0.0f, 1.0f, (float) intensity.getValue() / 110.0f);

    // Small decorative curve at the left.
    juce::Path leftCurve;
    leftCurve.startNewSubPath (w * 0.185f, h * 0.305f);
    leftCurve.quadraticTo (w * 0.215f, h * 0.50f, w * 0.255f, h * 0.305f);
    strokeRoundedPath (g, leftCurve, juce::Colour (0xffe2ded9), 4.0f);

    // Readout is separate from the slider itself (its JUCE text box is disabled).
    const float dialCX = w * 0.50f;
    const float dialWidthForLayout = (float) juce::jmin (310, juce::roundToInt (h * 0.44f));
    const float dialCenterY = h * 0.06f + (dialWidthForLayout + 30.0f) * 0.47f;
    const float readoutY = dialCenterY + dialWidthForLayout * 0.28f;
    const float labelY = dialCenterY + dialWidthForLayout * 0.47f;
    g.setColour (juce::Colour (0xffd1ccca).withAlpha (0.75f));
    g.drawRect (juce::Rectangle<float> (dialCX - 45.0f, readoutY, 90.0f, 24.0f), 1.0f);
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions (14.0f)));
    g.drawText (juce::String (intensity.getValue(), 2),
                juce::Rectangle<int> (juce::roundToInt (dialCX - 43.0f), juce::roundToInt (readoutY), 86, 24),
                juce::Justification::centred);
    g.setColour (juce::Colour (0xff101114));
    g.setFont (juce::Font (juce::FontOptions (15.0f, juce::Font::bold)));
    g.drawText ("INTENSITY",
                juce::Rectangle<int> (juce::roundToInt (dialCX - 70.0f), juce::roundToInt (labelY), 140, 22),
                juce::Justification::centred);

    // Original-inspired, clean-room smiling mascot with arms that lift as intensity rises.
    const float faceCX = w * 0.735f;
    const float faceCY = h * 0.465f;
    const float faceR = juce::jmin (w * 0.155f, h * 0.275f);

    juce::Path leftArm;
    leftArm.startNewSubPath (faceCX - faceR * 0.78f, faceCY - faceR * 0.48f);
    leftArm.lineTo (faceCX - faceR * (1.10f + 0.08f * t), faceCY - faceR * (1.03f + 0.08f * t));
    juce::Path rightArm;
    rightArm.startNewSubPath (faceCX + faceR * 0.78f, faceCY - faceR * 0.48f);
    rightArm.lineTo (faceCX + faceR * (1.10f + 0.08f * t), faceCY - faceR * (1.03f + 0.08f * t));
    strokeRoundedPath (g, leftArm, pink, 7.0f);
    strokeRoundedPath (g, rightArm, pink, 7.0f);

    g.setColour (orange);
    g.fillEllipse (faceCX - faceR * 1.03f, faceCY - faceR * 1.03f, faceR * 2.06f, faceR * 2.06f);
    g.setColour (juce::Colour (0xffd8f0df));
    g.fillEllipse (faceCX - faceR * 0.88f, faceCY - faceR * 0.88f, faceR * 1.76f, faceR * 1.76f);

    g.setColour (juce::Colour (0xffa42f59));
    const float eyeH = faceR * (0.30f + 0.05f * t);
    g.fillEllipse (faceCX - faceR * 0.47f, faceCY - faceR * 0.32f, faceR * 0.20f, eyeH);
    g.fillEllipse (faceCX + faceR * 0.27f, faceCY - faceR * 0.32f, faceR * 0.20f, eyeH);

    const float mouthWidth = faceR * (0.57f + 0.31f * t);
    const float mouthY = faceCY + faceR * 0.20f;
    const float mouthDrop = faceR * (0.11f + 0.28f * t);
    juce::Path smileShadow;
    smileShadow.startNewSubPath (faceCX - mouthWidth, mouthY + 3.0f);
    smileShadow.quadraticTo (faceCX, mouthY + mouthDrop * 2.0f + 3.0f,
                             faceCX + mouthWidth, mouthY + 3.0f);
    strokeRoundedPath (g, smileShadow, pinkShadow, faceR * 0.075f);
    juce::Path smile;
    smile.startNewSubPath (faceCX - mouthWidth, mouthY);
    smile.quadraticTo (faceCX, mouthY + mouthDrop * 2.0f,
                       faceCX + mouthWidth, mouthY);
    strokeRoundedPath (g, smile, pink, faceR * 0.060f);

    if (t > 0.62f)
    {
        juce::Path mouth;
        mouth.startNewSubPath (faceCX - mouthWidth * 0.70f, mouthY + 2.0f);
        mouth.quadraticTo (faceCX, mouthY + mouthDrop * 1.65f,
                           faceCX + mouthWidth * 0.70f, mouthY + 2.0f);
        mouth.lineTo (faceCX + mouthWidth * 0.55f, mouthY + mouthDrop * 1.42f);
        mouth.quadraticTo (faceCX, mouthY + mouthDrop * 1.64f,
                           faceCX - mouthWidth * 0.55f, mouthY + mouthDrop * 1.42f);
        mouth.closeSubPath();
        g.setColour (pinkShadow);
        g.fillPath (mouth);
        g.setColour (juce::Colour (0xffffd8cf));
        g.fillRoundedRectangle (faceCX - mouthWidth * 0.47f, mouthY + 4.0f,
                                mouthWidth * 0.94f, juce::jmax (4.0f, faceR * 0.065f), 3.0f);
    }

    g.setColour (juce::Colour (0xffd5d0c9).withAlpha (0.82f));
    g.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::bold)));
    g.drawText ("SKELLY AUDIO   |   BUILD-UP / TRANSITION EFFECT",
                20, getHeight() - 24, getWidth() - 40, 14, juce::Justification::centredLeft);
}

void EndlessSmileInspiredAudioProcessorEditor::resized()
{
    const auto bounds = getLocalBounds();
    const int dialWidth = juce::jmin (310, juce::roundToInt ((float) bounds.getHeight() * 0.44f));
    const int dialHeight = dialWidth + 30;
    const int dialX = juce::roundToInt ((float) bounds.getWidth() * 0.50f) - dialWidth / 2;
    const int dialY = juce::roundToInt ((float) bounds.getHeight() * 0.06f);
    intensity.setBounds (dialX, dialY, dialWidth, dialHeight);

    const int controlWidth = juce::jmin (660, juce::roundToInt ((float) bounds.getWidth() * 0.49f));
    const int presetY = juce::roundToInt ((float) bounds.getHeight() * 0.765f);
    preset.setBounds (juce::roundToInt ((float) bounds.getWidth() * 0.042f), presetY,
                      controlWidth, 40);
    mix.setBounds (juce::roundToInt ((float) bounds.getWidth() * 0.052f), presetY + 46,
                   controlWidth - 12, 30);
}

void EndlessSmileInspiredAudioProcessorEditor::timerCallback()
{
    repaint();
}
