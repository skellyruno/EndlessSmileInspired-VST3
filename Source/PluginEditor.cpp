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
    float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    juce::ignoreUnused (rotaryStartAngle, rotaryEndAngle);

    const float side = (float) juce::jmin (width, height);
    const float cx = (float) x + (float) width * 0.5f;
    // Keep the dial centre tied to its width so added readout space below
    // the knob does not push the visual dial down as the editor is resized.
    const float cy = (float) y + (float) width * 0.487f;
    const float radius = side * 0.455f;
    const float start = juce::MathConstants<float>::pi * 0.70f;
    const float end = juce::MathConstants<float>::pi * 2.30f;
    const float t = juce::jlimit (0.0f, 1.0f, sliderPosProportional);
    const float current = start + (end - start) * t;

    // Soft drop shadow separates the control from the flat grey panel.
    g.setColour (juce::Colours::black.withAlpha (0.16f));
    g.fillEllipse (cx - radius - 2.0f, cy - radius + 3.0f,
                   (radius + 2.0f) * 2.0f, (radius + 2.0f) * 2.0f);

    // Recessed track with tiny calibration ticks.
    juce::Path track;
    track.addCentredArc (cx, cy, radius, radius, 0.0f, start, end, true);
    strokeRoundedPath (g, track, juce::Colour (0xff1e2a2e), 7.2f);

    for (int tick = 0; tick <= 20; ++tick)
    {
        const float a = start + (end - start) * (float) tick / 20.0f;
        const float inner = radius - ((tick % 5 == 0) ? 12.0f : 7.0f);
        const float outer = radius - 2.0f;
        const float x1 = cx + std::cos (a) * inner;
        const float y1 = cy + std::sin (a) * inner;
        const float x2 = cx + std::cos (a) * outer;
        const float y2 = cy + std::sin (a) * outer;
        g.setColour (juce::Colour (0xffc2c3c3).withAlpha (tick % 5 == 0 ? 0.56f : 0.23f));
        g.drawLine (x1, y1, x2, y2, tick % 5 == 0 ? 1.4f : 0.8f);
    }

    if (t > 0.0001f)
    {
        juce::Path active;
        active.addCentredArc (cx, cy, radius, radius, 0.0f, start, current, true);
        strokeRoundedPath (g, active, juce::Colour (0xff168ba9).withAlpha (0.42f), 10.0f);
        strokeRoundedPath (g, active, juce::Colour (0xff39afd2), 6.5f);
    }

    const float markerX = cx + std::cos (current) * radius;
    const float markerY = cy + std::sin (current) * radius;
    g.setColour (juce::Colour (0xff151d20).withAlpha (0.6f));
    g.fillEllipse (markerX - 7.6f, markerY - 7.6f, 15.2f, 15.2f);
    g.setColour (juce::Colour (0xff39afd2));
    g.fillEllipse (markerX - 5.8f, markerY - 5.8f, 11.6f, 11.6f);

    // Brushed-metal bezel and radial-gradient knob body.
    const float knobR = radius * 0.535f;
    g.setColour (juce::Colour (0xffd1d0d0).withAlpha (0.94f));
    g.fillEllipse (cx - knobR - 3.2f, cy - knobR - 3.2f,
                   (knobR + 3.2f) * 2.0f, (knobR + 3.2f) * 2.0f);
    g.setColour (juce::Colour (0xff777879));
    g.drawEllipse (cx - knobR - 1.5f, cy - knobR - 1.5f,
                   (knobR + 1.5f) * 2.0f, (knobR + 1.5f) * 2.0f, 1.0f);

    juce::ColourGradient body (juce::Colour (0xff55575a), cx - knobR * 0.48f, cy - knobR * 0.70f,
                               juce::Colour (0xff101214), cx + knobR * 0.78f, cy + knobR * 0.78f, true);
    body.addColour (0.38, juce::Colour (0xff292b2e));
    body.addColour (0.72, juce::Colour (0xff17191b));
    g.setGradientFill (body);
    g.fillEllipse (cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f);

    // Subtle glints on the upper edge of the metal ring.
    juce::Path bezelGlint;
    bezelGlint.addCentredArc (cx, cy, knobR + 1.0f, knobR + 1.0f, 0.0f,
                              juce::MathConstants<float>::pi * 1.08f,
                              juce::MathConstants<float>::pi * 1.88f, true);
    strokeRoundedPath (g, bezelGlint, juce::Colours::white.withAlpha (0.28f), 1.4f);

    // Angled pointer with a slim shadow and bright center line.
    const float pointerAngle = juce::MathConstants<float>::pi * (0.72f + 1.08f * t);
    const float px = std::cos (pointerAngle) * knobR * 0.73f;
    const float py = std::sin (pointerAngle) * knobR * 0.73f;
    g.setColour (juce::Colours::black.withAlpha (0.48f));
    g.drawLine (cx + 1.0f, cy + 1.4f, cx + px + 1.0f, cy + py + 1.4f, 3.0f);
    g.setColour (juce::Colour (0xffe7e8e8).withAlpha (0.92f));
    g.drawLine (cx, cy, cx + px, cy + py, 2.0f);
    g.setColour (juce::Colour (0xff8d9294).withAlpha (0.65f));
    g.fillEllipse (cx - 2.4f, cy - 2.4f, 4.8f, 4.8f);

    // Render the readout as part of the dial component, after the arc and knob.
    // This avoids the child slider being painted over by its parent's background.
    const float readoutY = cy + radius + 8.0f;
    g.setColour (juce::Colour (0xff282629).withAlpha (0.16f));
    g.fillRoundedRectangle (cx - 46.0f, readoutY + 1.0f, 92.0f, 24.0f, 1.5f);
    g.setColour (juce::Colour (0xffd4d0d0).withAlpha (0.82f));
    g.drawRoundedRectangle (cx - 45.0f, readoutY, 90.0f, 24.0f, 1.0f, 0.9f);
    g.setColour (juce::Colour (0xfffbfafa));
    g.setFont (juce::Font (juce::FontOptions (13.5f)));
    g.drawText (juce::String (slider.getValue(), 2),
                juce::Rectangle<int> (juce::roundToInt (cx - 43.0f),
                                      juce::roundToInt (readoutY), 86, 24),
                juce::Justification::centred);

    g.setColour (juce::Colour (0xff171719));
    g.setFont (juce::Font (juce::FontOptions (14.5f, juce::Font::bold)));
    g.drawText ("INTENSITY",
                juce::Rectangle<int> (juce::roundToInt (cx - 72.0f),
                                      juce::roundToInt (readoutY + 25.0f), 144, 20),
                juce::Justification::centred);
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
    const float intensityValue = (float) intensity.getValue();
    const float t = juce::jlimit (0.0f, 1.0f, intensityValue / 110.0f);

    // Soft charcoal-grey panel gradient with a quiet vignette.
    juce::ColourGradient background (juce::Colour (0xff6b696a), 0.0f, 0.0f,
                                    juce::Colour (0xff5f5d5e), w, h, false);
    background.addColour (0.52, juce::Colour (0xff686667));
    g.setGradientFill (background);
    g.fillAll();

    // Deterministic low-contrast panel grain avoids flicker while repainting.
    juce::Random random (91);
    for (int i = 0; i < 430; ++i)
    {
        const float px = random.nextFloat() * w;
        const float py = random.nextFloat() * h;
        g.setColour ((i % 3 == 0 ? juce::Colours::white : juce::Colours::black)
                     .withAlpha (random.nextFloat() * 0.018f));
        g.fillEllipse (px, py, 1.0f, 1.0f);
    }

    // Double inset border with a slight highlight along the top edge.
    g.setColour (juce::Colour (0xff252427));
    g.drawRoundedRectangle (bounds.reduced (8.0f), 11.0f, 3.0f);
    g.setColour (juce::Colour (0xffd2cece).withAlpha (0.66f));
    g.drawRoundedRectangle (bounds.reduced (13.0f), 8.0f, 1.2f);
    juce::Path topGlint;
    topGlint.startNewSubPath (18.0f, 20.0f);
    topGlint.lineTo (w - 18.0f, 20.0f);
    strokeRoundedPath (g, topGlint, juce::Colours::white.withAlpha (0.09f), 1.0f);

    // Clean-room smile mascot. Shadow, orange rim, mint face, and face details
    // are layered deliberately so the mouth stays organic at every intensity.
    const float faceCX = w * 0.748f;
    const float faceCY = h * 0.462f;
    const float faceR = juce::jmin (w * 0.151f, h * 0.267f);

    // Small segmented torso peeks out below the head, like a spring-loaded toy.
    // Draw it first so the face naturally overlaps the top of the neck.
    const float torsoTop = faceCY + faceR * 0.65f;
    const float torsoBottom = h * 0.895f;
    juce::Path torso;
    torso.startNewSubPath (faceCX - faceR * 0.34f, torsoTop);
    torso.cubicTo (faceCX - faceR * 0.43f, torsoTop + faceR * 0.24f,
                   faceCX - faceR * 0.30f, torsoBottom - faceR * 0.16f,
                   faceCX - faceR * 0.23f, torsoBottom);
    torso.cubicTo (faceCX - faceR * 0.18f, torsoBottom + faceR * 0.018f,
                   faceCX + faceR * 0.18f, torsoBottom + faceR * 0.018f,
                   faceCX + faceR * 0.23f, torsoBottom);
    torso.cubicTo (faceCX + faceR * 0.30f, torsoBottom - faceR * 0.16f,
                   faceCX + faceR * 0.43f, torsoTop + faceR * 0.24f,
                   faceCX + faceR * 0.34f, torsoTop);
    torso.closeSubPath();
    juce::ColourGradient torsoGradient (juce::Colour (0xffff73a9), faceCX - faceR * 0.3f, torsoTop,
                                        juce::Colour (0xffce3672), faceCX + faceR * 0.35f, torsoBottom, false);
    g.setGradientFill (torsoGradient);
    g.fillPath (torso);
    g.setColour (juce::Colour (0xff842746).withAlpha (0.65f));
    for (int band = 0; band < 4; ++band)
    {
        const float y = torsoTop + faceR * (0.20f + (float) band * 0.18f);
        juce::Path seam;
        seam.startNewSubPath (faceCX - faceR * (0.31f - (float) band * 0.020f), y);
        seam.quadraticTo (faceCX, y + faceR * 0.055f,
                          faceCX + faceR * (0.31f - (float) band * 0.020f), y);
        strokeRoundedPath (g, seam, juce::Colour (0xff9e2d5b).withAlpha (0.75f), 1.3f);
        strokeRoundedPath (g, seam, juce::Colour (0xffffa0bf).withAlpha (0.58f), 0.8f);
    }

    // Raised arms now arc up from behind the head instead of stopping at its temples.
    // Their height responds gently to intensity while hands remain visible at every setting.
    const float handY = h * (0.145f - 0.075f * t);
    const float leftShoulderX = faceCX - faceR * 0.66f;
    const float rightShoulderX = faceCX + faceR * 0.66f;
    const float shoulderY = faceCY + faceR * 0.51f;
    const float leftHandX = faceCX - faceR * 1.16f;
    const float rightHandX = faceCX + faceR * 1.16f;

    juce::Path leftArm;
    leftArm.startNewSubPath (leftShoulderX, shoulderY);
    leftArm.cubicTo (faceCX - faceR * 1.02f, faceCY + faceR * 0.42f,
                     leftHandX - faceR * 0.20f, handY + faceR * 0.70f,
                     leftHandX, handY);
    juce::Path rightArm;
    rightArm.startNewSubPath (rightShoulderX, shoulderY);
    rightArm.cubicTo (faceCX + faceR * 1.02f, faceCY + faceR * 0.42f,
                      rightHandX + faceR * 0.20f, handY + faceR * 0.70f,
                      rightHandX, handY);
    strokeRoundedPath (g, leftArm, juce::Colour (0xff812746), 10.5f);
    strokeRoundedPath (g, rightArm, juce::Colour (0xff812746), 10.5f);
    strokeRoundedPath (g, leftArm, juce::Colour (0xfff04491), 7.4f);
    strokeRoundedPath (g, rightArm, juce::Colour (0xfff04491), 7.4f);
    strokeRoundedPath (g, leftArm, juce::Colour (0xffff9cc6).withAlpha (0.62f), 1.2f);
    strokeRoundedPath (g, rightArm, juce::Colour (0xffff9cc6).withAlpha (0.62f), 1.2f);

    // Simple mint hands and separated fingers; not intensity-gated so the character never loses its hands.
    const float handW = juce::jmax (8.0f, faceR * 0.075f);
    const float handH = juce::jmax (11.0f, faceR * 0.105f);
    for (int side = 0; side < 2; ++side)
    {
        const float hx = side == 0 ? leftHandX : rightHandX;
        g.setColour (juce::Colour (0xffd8f0df));
        g.fillRoundedRectangle (hx - handW * 0.5f, handY - handH * 0.25f, handW, handH, 2.5f);
        g.setColour (juce::Colour (0xffa7d6bf));
        g.drawRoundedRectangle (hx - handW * 0.5f, handY - handH * 0.25f, handW, handH, 2.5f, 0.8f);
        g.setColour (juce::Colour (0xfff2eee6));
        for (int finger = -1; finger <= 1; ++finger)
        {
            const float fx = hx + (float) finger * handW * 0.31f;
            const float topY = handY - handH * (0.56f + (finger == 0 ? 0.25f : 0.12f));
            g.drawLine (fx, handY - handH * 0.10f, fx, topY, 1.2f);
            g.fillEllipse (fx - 0.9f, topY - 0.9f, 1.8f, 1.8f);
        }
    }

    // A soft shadow makes the orange rim appear raised from the panel.
    g.setColour (juce::Colours::black.withAlpha (0.16f));
    g.fillEllipse (faceCX - faceR * 1.04f + 2.0f, faceCY - faceR * 1.01f + 4.0f,
                   faceR * 2.08f, faceR * 2.08f);
    g.setColour (juce::Colour (0xffff9b2f));
    g.fillEllipse (faceCX - faceR * 1.03f, faceCY - faceR * 1.03f,
                   faceR * 2.06f, faceR * 2.06f);
    g.setColour (juce::Colour (0xffffc05d).withAlpha (0.82f));
    juce::Path rimHighlight;
    rimHighlight.addCentredArc (faceCX, faceCY, faceR * 0.985f, faceR * 0.985f, 0.0f,
                                juce::MathConstants<float>::pi * 1.08f,
                                juce::MathConstants<float>::pi * 1.88f, true);
    strokeRoundedPath (g, rimHighlight, juce::Colour (0xffffc15c), 2.2f);

    juce::ColourGradient faceGradient (juce::Colour (0xffe6f7e9), faceCX - faceR * 0.52f,
                                       faceCY - faceR * 0.72f,
                                       juce::Colour (0xffcce8d5), faceCX + faceR * 0.55f,
                                       faceCY + faceR * 0.85f, false);
    faceGradient.addColour (0.40, juce::Colour (0xffd9f0df));
    g.setGradientFill (faceGradient);
    g.fillEllipse (faceCX - faceR * 0.88f, faceCY - faceR * 0.88f,
                   faceR * 1.76f, faceR * 1.76f);

    // Eyes: smooth ovals with a restrained highlight for a more finished mascot.
    const float eyeW = faceR * (0.19f + 0.012f * t);
    const float eyeH = faceR * (0.29f + 0.025f * t);
    const float eyeY = faceCY - faceR * 0.33f;
    const float leftEyeX = faceCX - faceR * 0.48f;
    const float rightEyeX = faceCX + faceR * 0.28f;
    g.setColour (juce::Colour (0xffa42f59));
    g.fillEllipse (leftEyeX, eyeY, eyeW, eyeH);
    g.fillEllipse (rightEyeX, eyeY, eyeW, eyeH);
    g.setColour (juce::Colour (0xffffc8d9).withAlpha (0.65f));
    g.fillEllipse (leftEyeX + eyeW * 0.22f, eyeY + eyeH * 0.12f,
                   eyeW * 0.20f, eyeH * 0.19f);
    g.fillEllipse (rightEyeX + eyeW * 0.22f, eyeY + eyeH * 0.12f,
                   eyeW * 0.20f, eyeH * 0.19f);

    // A little cheek warmth appears as the intensity rises.
    if (t > 0.20f)
    {
        g.setColour (juce::Colour (0xffff8bae).withAlpha (0.10f + 0.12f * t));
        g.fillEllipse (faceCX - faceR * 0.70f, faceCY + faceR * 0.10f,
                       faceR * 0.30f, faceR * 0.16f);
        g.fillEllipse (faceCX + faceR * 0.40f, faceCY + faceR * 0.10f,
                       faceR * 0.30f, faceR * 0.16f);
    }

    // A softly shaded toy nose gives the face a stronger silhouette without hiding the eyes.
    const float noseTop = faceCY - faceR * 0.29f;
    const float noseBottom = faceCY + faceR * 0.075f;
    const float noseHalfWidth = faceR * (0.13f + 0.012f * t);
    juce::Path nose;
    nose.startNewSubPath (faceCX - noseHalfWidth * 0.62f, noseTop + faceR * 0.025f);
    nose.cubicTo (faceCX - noseHalfWidth * 1.16f, noseTop + faceR * 0.10f,
                  faceCX - noseHalfWidth * 0.94f, noseBottom - faceR * 0.025f,
                  faceCX - noseHalfWidth * 0.30f, noseBottom);
    nose.cubicTo (faceCX + noseHalfWidth * 0.30f, noseBottom + faceR * 0.018f,
                  faceCX + noseHalfWidth * 0.94f, noseBottom - faceR * 0.085f,
                  faceCX + noseHalfWidth * 0.62f, noseTop + faceR * 0.025f);
    nose.closeSubPath();
    juce::ColourGradient noseGradient (juce::Colour (0xffff8bc0), faceCX - noseHalfWidth, noseTop,
                                       juce::Colour (0xffb43a70), faceCX + noseHalfWidth, noseBottom, false);
    noseGradient.addColour (0.42, juce::Colour (0xffdf6299));
    g.setGradientFill (noseGradient);
    g.fillPath (nose);
    g.setColour (juce::Colour (0xffffc0d8).withAlpha (0.78f));
    g.drawLine (faceCX - noseHalfWidth * 0.22f, noseTop + faceR * 0.065f,
                faceCX - noseHalfWidth * 0.16f, noseBottom - faceR * 0.085f, 1.2f);
    g.setColour (juce::Colour (0xff842746).withAlpha (0.48f));
    g.fillEllipse (faceCX - noseHalfWidth * 0.46f, noseBottom - faceR * 0.065f,
                   noseHalfWidth * 0.25f, faceR * 0.025f);
    g.fillEllipse (faceCX + noseHalfWidth * 0.19f, noseBottom - faceR * 0.065f,
                   noseHalfWidth * 0.25f, faceR * 0.025f);

    const float mouthHalfWidth = faceR * (0.56f + 0.20f * t);
    const float mouthTopY = faceCY + faceR * (0.20f + 0.015f * t);
    const float mouthDepth = faceR * (0.10f + 0.34f * t);

    if (t < 0.48f)
    {
        // Low intensity: thick, round, happy smile with a dark lower edge.
        juce::Path smileShade;
        smileShade.startNewSubPath (faceCX - mouthHalfWidth, mouthTopY + 2.2f);
        smileShade.cubicTo (faceCX - mouthHalfWidth * 0.44f, mouthTopY + mouthDepth * 1.95f,
                            faceCX + mouthHalfWidth * 0.44f, mouthTopY + mouthDepth * 1.95f,
                            faceCX + mouthHalfWidth, mouthTopY + 2.2f);
        strokeRoundedPath (g, smileShade, juce::Colour (0xff812746), faceR * 0.085f);
        juce::Path smile;
        smile.startNewSubPath (faceCX - mouthHalfWidth, mouthTopY);
        smile.cubicTo (faceCX - mouthHalfWidth * 0.44f, mouthTopY + mouthDepth * 1.85f,
                       faceCX + mouthHalfWidth * 0.44f, mouthTopY + mouthDepth * 1.85f,
                       faceCX + mouthHalfWidth, mouthTopY);
        strokeRoundedPath (g, smile, juce::Colour (0xfff04491), faceR * 0.060f);
    }
    else
    {
        // High intensity: smooth open crescent mouth, no sharp triangular corners.
        const float topCurveDepth = faceR * (0.13f + 0.055f * t);
        const float mouthBottomY = mouthTopY + mouthDepth;
        juce::Path mouth;
        mouth.startNewSubPath (faceCX - mouthHalfWidth, mouthTopY);
        mouth.cubicTo (faceCX - mouthHalfWidth * 0.54f, mouthTopY + topCurveDepth,
                       faceCX + mouthHalfWidth * 0.54f, mouthTopY + topCurveDepth,
                       faceCX + mouthHalfWidth, mouthTopY);
        mouth.cubicTo (faceCX + mouthHalfWidth * 0.83f, mouthBottomY - mouthDepth * 0.10f,
                       faceCX + mouthHalfWidth * 0.43f, mouthBottomY,
                       faceCX, mouthBottomY);
        mouth.cubicTo (faceCX - mouthHalfWidth * 0.43f, mouthBottomY,
                       faceCX - mouthHalfWidth * 0.83f, mouthBottomY - mouthDepth * 0.10f,
                       faceCX - mouthHalfWidth, mouthTopY);
        mouth.closeSubPath();
        g.setColour (juce::Colour (0xff812746));
        g.fillPath (mouth);
        // Inner mouth tint softens the large dark area while retaining a bold grin.
        juce::Path innerMouth;
        innerMouth.startNewSubPath (faceCX - mouthHalfWidth * 0.78f, mouthTopY + topCurveDepth * 0.74f);
        innerMouth.cubicTo (faceCX - mouthHalfWidth * 0.45f, mouthTopY + topCurveDepth * 1.22f,
                            faceCX + mouthHalfWidth * 0.45f, mouthTopY + topCurveDepth * 1.22f,
                            faceCX + mouthHalfWidth * 0.78f, mouthTopY + topCurveDepth * 0.74f);
        innerMouth.cubicTo (faceCX + mouthHalfWidth * 0.60f, mouthTopY + mouthDepth * 0.67f,
                            faceCX - mouthHalfWidth * 0.60f, mouthTopY + mouthDepth * 0.67f,
                            faceCX - mouthHalfWidth * 0.78f, mouthTopY + topCurveDepth * 0.74f);
        innerMouth.closeSubPath();
        g.setColour (juce::Colour (0xff9f3159).withAlpha (0.55f));
        g.fillPath (innerMouth);

        // Five connected-looking upper teeth sit inside the mouth, following the lip's arc.
        // Centers and widths stay inside the mouth silhouette at all editor sizes.
        const float teethW = mouthHalfWidth * 0.245f;
        const float teethH = juce::jmax (5.0f, faceR * (0.075f + 0.016f * t));
        for (int tooth = 0; tooth < 5; ++tooth)
        {
            const float normalizedX = ((float) tooth - 2.0f) / 2.0f;
            const float tx = faceCX + normalizedX * mouthHalfWidth * 0.69f;
            const float xFromCentre = std::abs (normalizedX * 0.69f);
            // The top lip is a cubic arch: use its curved envelope so corner teeth
            // stay attached to the mouth instead of appearing to float on the cheeks.
            const float topCurveAtX = 0.75f * (1.0f - xFromCentre * xFromCentre);
            const float ty = mouthTopY + topCurveDepth * topCurveAtX + 2.0f;
            const float toothWidth = teethW * (tooth == 2 ? 1.12f : 1.0f);
            g.setColour (juce::Colour (0xff6f2943).withAlpha (0.38f));
            g.fillRoundedRectangle (tx - toothWidth * 0.5f + 0.6f, ty + 1.1f,
                                    toothWidth, teethH, 2.6f);
            g.setColour (juce::Colour (0xffffe8df));
            g.fillRoundedRectangle (tx - toothWidth * 0.5f, ty,
                                    toothWidth, teethH, 2.6f);
            g.setColour (juce::Colour (0xfffffff7).withAlpha (0.72f));
            g.drawLine (tx - toothWidth * 0.28f, ty + 1.4f,
                        tx + toothWidth * 0.18f, ty + 1.4f, 0.8f);
        }

        // A smaller lower row near the corners balances the big grin but leaves room for the tongue.
        const float lowerToothW = mouthHalfWidth * 0.145f;
        const float lowerToothH = juce::jmax (4.0f, faceR * 0.047f);
        for (int tooth = 0; tooth < 4; ++tooth)
        {
            const float side = tooth < 2 ? -1.0f : 1.0f;
            const float position = (tooth % 2 == 0) ? 0.68f : 0.46f;
            const float tx = faceCX + side * mouthHalfWidth * position;
            const float ty = mouthTopY + mouthDepth * (0.73f - 0.035f * position);
            g.setColour (juce::Colour (0xff6f2943).withAlpha (0.35f));
            g.fillRoundedRectangle (tx - lowerToothW * 0.5f + 0.5f, ty + 0.8f,
                                    lowerToothW, lowerToothH, 2.0f);
            g.setColour (juce::Colour (0xffffe8df));
            g.fillRoundedRectangle (tx - lowerToothW * 0.5f, ty,
                                    lowerToothW, lowerToothH, 2.0f);
        }

        // Tongue is a gentle pink capsule near the lower curve on intense settings.
        if (t > 0.80f)
        {
            g.setColour (juce::Colour (0xffec6697).withAlpha (0.95f));
            g.fillRoundedRectangle (faceCX - mouthHalfWidth * 0.25f,
                                    mouthTopY + mouthDepth * 0.72f,
                                    mouthHalfWidth * 0.50f, faceR * 0.085f, faceR * 0.04f);
            g.setColour (juce::Colour (0xffad4166).withAlpha (0.7f));
            g.drawLine (faceCX, mouthTopY + mouthDepth * 0.74f,
                        faceCX, mouthTopY + mouthDepth * 0.83f, 0.9f);
        }
    }

    // Footer branding kept subtle so the controls and mascot remain the focus.
    g.setColour (juce::Colour (0xffd5d0c9).withAlpha (0.84f));
    g.setFont (juce::Font (juce::FontOptions (9.5f, juce::Font::bold)));
    g.drawText ("SKELLY AUDIO  |  BUILD-UP / TRANSITION EFFECT",
                20, getHeight() - 24, getWidth() - 40, 14, juce::Justification::centredLeft);
}

void EndlessSmileInspiredAudioProcessorEditor::resized()
{
    const auto bounds = getLocalBounds();
    const int dialWidth = juce::jmin (310, juce::roundToInt ((float) bounds.getHeight() * 0.44f));
    const int dialHeight = dialWidth + 50;
    const int dialX = juce::roundToInt ((float) bounds.getWidth() * 0.47f) - dialWidth / 2;
    const int dialY = juce::roundToInt ((float) bounds.getHeight() * 0.06f);
    intensity.setBounds (dialX, dialY, dialWidth, dialHeight);

    const int leftMargin = juce::roundToInt ((float) bounds.getWidth() * 0.044f);
    const int controlWidth = juce::jmin (660, juce::roundToInt ((float) bounds.getWidth() * 0.49f));
    const int presetY = juce::roundToInt ((float) bounds.getHeight() * 0.765f);
    preset.setBounds (leftMargin, presetY, controlWidth, 40);
    mix.setBounds (leftMargin + 10, presetY + 46, controlWidth - 22, 30);
}

void EndlessSmileInspiredAudioProcessorEditor::timerCallback()
{
    repaint();
}
