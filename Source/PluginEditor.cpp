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

    // Finished charcoal panel: quiet centre lift, dark edges, subtle deterministic grain.
    juce::ColourGradient background (juce::Colour (0xff6d6a6b), w * 0.46f, h * 0.33f,
                                     juce::Colour (0xff5a585a), w, h, true);
    background.addColour (0.55, juce::Colour (0xff686667));
    g.setGradientFill (background);
    g.fillAll();

    juce::Random random (91);
    for (int i = 0; i < 360; ++i)
    {
        const float px = random.nextFloat() * w;
        const float py = random.nextFloat() * h;
        const float alpha = random.nextFloat() * 0.014f;
        g.setColour ((i % 3 == 0 ? juce::Colours::white : juce::Colours::black).withAlpha (alpha));
        g.fillEllipse (px, py, 1.0f, 1.0f);
    }

    g.setColour (juce::Colour (0xff262427));
    g.drawRoundedRectangle (bounds.reduced (8.0f), 11.0f, 3.0f);
    g.setColour (juce::Colour (0xffd2cece).withAlpha (0.72f));
    g.drawRoundedRectangle (bounds.reduced (13.0f), 8.0f, 1.2f);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawLine (20.0f, 21.0f, w - 20.0f, 21.0f, 1.0f);

    // The character is drawn behind the real JUCE intensity slider. This keeps
    // arms behind the dial and gives the whole mascot a consistent silhouette.
    const float faceCX = w * 0.748f;
    const float faceCY = h * 0.462f;
    const float faceR = juce::jmin (w * 0.151f, h * 0.267f);
    const float headBottom = faceCY + faceR * 1.03f;
    const float torsoTop = faceCY + faceR * 0.69f;
    const float torsoBottom = h * 0.832f;
    const float legBottom = h * 0.906f;
    const float shoeBottom = h * 0.936f;

    // Little legs and sneakers: the former dress-like torso now reads as a toy body.
    for (int side = -1; side <= 1; side += 2)
    {
        const float legX = faceCX + (float) side * faceR * 0.17f;
        g.setColour (juce::Colours::black.withAlpha (0.25f));
        g.drawLine (legX + 1.0f, torsoBottom - 2.0f, legX + 1.0f, legBottom, 10.0f);
        g.setColour (juce::Colour (0xfff04491));
        g.drawLine (legX, torsoBottom - 4.0f, legX, legBottom, 7.0f);
        g.setColour (juce::Colour (0xffd8f0df));
        g.drawLine (legX, legBottom - 6.0f, legX, legBottom, 5.0f);

        const float shoeW = faceR * 0.31f;
        const float shoeH = shoeBottom - legBottom;
        juce::Path shoe;
        if (side < 0)
        {
            shoe.startNewSubPath (legX - 7.0f, legBottom - 2.0f);
            shoe.cubicTo (legX - shoeW * 0.30f, legBottom + shoeH * 0.15f,
                          legX - shoeW * 0.68f, shoeBottom - shoeH * 0.22f,
                          legX - shoeW, shoeBottom - shoeH * 0.08f);
            shoe.cubicTo (legX - shoeW * 1.05f, shoeBottom,
                          legX - shoeW * 0.12f, shoeBottom + 1.0f,
                          legX + 5.0f, shoeBottom - 1.0f);
        }
        else
        {
            shoe.startNewSubPath (legX + 7.0f, legBottom - 2.0f);
            shoe.cubicTo (legX + shoeW * 0.30f, legBottom + shoeH * 0.15f,
                          legX + shoeW * 0.68f, shoeBottom - shoeH * 0.22f,
                          legX + shoeW, shoeBottom - shoeH * 0.08f);
            shoe.cubicTo (legX + shoeW * 1.05f, shoeBottom,
                          legX + shoeW * 0.12f, shoeBottom + 1.0f,
                          legX - 5.0f, shoeBottom - 1.0f);
        }
        shoe.closeSubPath();
        g.setColour (juce::Colour (0xff872746));
        g.fillPath (shoe, juce::AffineTransform::translation (0.0f, 2.0f));
        g.setColour (juce::Colour (0xfff44b96));
        g.fillPath (shoe);
        g.setColour (juce::Colour (0xffffbfd5).withAlpha (0.86f));
        juce::Path shoeStripe;
        shoeStripe.startNewSubPath (legX - shoeW * 0.48f, legBottom + shoeH * 0.43f);
        shoeStripe.lineTo (legX + shoeW * 0.10f, legBottom + shoeH * 0.64f);
        strokeRoundedPath (g, shoeStripe, juce::Colour (0xffffbfd5), 2.0f);
    }

    // Rounded, tapered spring torso, with a shoulder collar and four ribbed bands.
    juce::Path torso;
    torso.startNewSubPath (faceCX - faceR * 0.34f, torsoTop);
    torso.cubicTo (faceCX - faceR * 0.44f, torsoTop + faceR * 0.18f,
                   faceCX - faceR * 0.32f, torsoBottom - faceR * 0.16f,
                   faceCX - faceR * 0.25f, torsoBottom);
    torso.cubicTo (faceCX - faceR * 0.14f, torsoBottom + 2.0f,
                   faceCX + faceR * 0.14f, torsoBottom + 2.0f,
                   faceCX + faceR * 0.25f, torsoBottom);
    torso.cubicTo (faceCX + faceR * 0.32f, torsoBottom - faceR * 0.16f,
                   faceCX + faceR * 0.44f, torsoTop + faceR * 0.18f,
                   faceCX + faceR * 0.34f, torsoTop);
    torso.closeSubPath();
    g.setColour (juce::Colours::black.withAlpha (0.18f));
    g.fillPath (torso, juce::AffineTransform::translation (2.0f, 3.0f));
    juce::ColourGradient torsoGradient (juce::Colour (0xffff83b2), faceCX - faceR * 0.30f, torsoTop,
                                        juce::Colour (0xffca2f6b), faceCX + faceR * 0.33f, torsoBottom, false);
    torsoGradient.addColour (0.47, juce::Colour (0xffed4b8f));
    g.setGradientFill (torsoGradient);
    g.fillPath (torso);

    // A thick collar separates the head from the ribbed spring body.
    juce::Path collar;
    collar.startNewSubPath (faceCX - faceR * 0.34f, torsoTop + faceR * 0.025f);
    collar.quadraticTo (faceCX, torsoTop + faceR * 0.13f,
                        faceCX + faceR * 0.34f, torsoTop + faceR * 0.025f);
    strokeRoundedPath (g, collar, juce::Colour (0xff9b2d58), 5.0f);
    strokeRoundedPath (g, collar, juce::Colour (0xffff8fba).withAlpha (0.75f), 2.0f);

    for (int band = 0; band < 4; ++band)
    {
        const float u = 0.20f + (float) band * 0.19f;
        const float y = torsoTop + (torsoBottom - torsoTop) * u;
        const float halfW = faceR * (0.31f - (float) band * 0.018f);
        juce::Path seam;
        seam.startNewSubPath (faceCX - halfW, y);
        seam.cubicTo (faceCX - halfW * 0.42f, y + faceR * 0.065f,
                      faceCX + halfW * 0.42f, y + faceR * 0.065f,
                      faceCX + halfW, y);
        strokeRoundedPath (g, seam, juce::Colour (0xffa52859).withAlpha (0.76f), 1.5f);
        strokeRoundedPath (g, seam, juce::Colour (0xffff9fc2).withAlpha (0.56f), 0.8f);
    }

    // Raised arms: shadow, dark outline, saturated pink tube and narrow glossy edge.
    const float handY = h * (0.105f - 0.012f * t);
    const float shoulderY = faceCY + faceR * 0.47f;
    const float leftShoulderX = faceCX - faceR * 0.67f;
    const float rightShoulderX = faceCX + faceR * 0.67f;
    const float leftHandX = faceCX - faceR * (1.18f - 0.015f * t);
    const float rightHandX = faceCX + faceR * (1.18f - 0.015f * t);

    juce::Path leftArm;
    leftArm.startNewSubPath (leftShoulderX, shoulderY);
    leftArm.cubicTo (faceCX - faceR * 1.00f, faceCY + faceR * 0.27f,
                     leftHandX - faceR * 0.19f, handY + faceR * 0.76f,
                     leftHandX, handY);
    juce::Path rightArm;
    rightArm.startNewSubPath (rightShoulderX, shoulderY);
    rightArm.cubicTo (faceCX + faceR * 1.00f, faceCY + faceR * 0.27f,
                      rightHandX + faceR * 0.19f, handY + faceR * 0.76f,
                      rightHandX, handY);
    strokeRoundedPath (g, leftArm, juce::Colours::black.withAlpha (0.24f), 14.0f);
    strokeRoundedPath (g, rightArm, juce::Colours::black.withAlpha (0.24f), 14.0f);
    strokeRoundedPath (g, leftArm, juce::Colour (0xff812746), 11.3f);
    strokeRoundedPath (g, rightArm, juce::Colour (0xff812746), 11.3f);
    strokeRoundedPath (g, leftArm, juce::Colour (0xfff04491), 8.2f);
    strokeRoundedPath (g, rightArm, juce::Colour (0xfff04491), 8.2f);
    strokeRoundedPath (g, leftArm, juce::Colour (0xffffb0d0).withAlpha (0.70f), 1.25f);
    strokeRoundedPath (g, rightArm, juce::Colour (0xffffb0d0).withAlpha (0.70f), 1.25f);

    // Hands: palms, four narrow fingers and an outward thumb, all in mint enamel.
    const float handW = juce::jmax (9.0f, faceR * 0.072f);
    const float handH = juce::jmax (10.0f, faceR * 0.085f);
    for (int side = 0; side < 2; ++side)
    {
        const float hx = side == 0 ? leftHandX : rightHandX;
        g.setColour (juce::Colour (0xff9e2e5a));
        g.fillRoundedRectangle (hx - handW * 0.52f, handY - handH * 0.30f + 1.0f,
                                handW * 1.04f, handH, 2.4f);
        g.setColour (juce::Colour (0xffd8f0df));
        g.fillRoundedRectangle (hx - handW * 0.50f, handY - handH * 0.30f,
                                handW, handH, 2.4f);
        g.setColour (juce::Colour (0xfff6f2e9));
        for (int finger = -1; finger <= 2; ++finger)
        {
            const float fx = hx + ((float) finger - 0.5f) * handW * 0.23f;
            const float fingerTop = handY - handH * (0.78f + ((finger == 0 || finger == 1) ? 0.30f : 0.13f));
            g.drawLine (fx, handY - handH * 0.14f, fx, fingerTop, 1.25f);
            g.fillEllipse (fx - 0.8f, fingerTop - 0.8f, 1.6f, 1.6f);
        }
        const float thumbDir = side == 0 ? -1.0f : 1.0f;
        g.drawLine (hx + thumbDir * handW * 0.34f, handY + handH * 0.12f,
                    hx + thumbDir * handW * 0.64f, handY - handH * 0.07f, 1.45f);
    }

    // Head drop shadow and subtle dark undershadow to lift the orange shell.
    g.setColour (juce::Colours::black.withAlpha (0.20f));
    g.fillEllipse (faceCX - faceR * 1.035f + 2.0f, faceCY - faceR * 1.02f + 5.0f,
                   faceR * 2.07f, faceR * 2.07f);
    g.setColour (juce::Colour (0xff8b5125).withAlpha (0.75f));
    g.drawEllipse (faceCX - faceR * 1.025f, faceCY - faceR * 1.025f,
                   faceR * 2.05f, faceR * 2.05f, 2.5f);
    juce::ColourGradient shell (juce::Colour (0xffffb044), faceCX - faceR * 0.83f, faceCY - faceR,
                                juce::Colour (0xfff18520), faceCX + faceR * 0.78f, faceCY + faceR, false);
    shell.addColour (0.42, juce::Colour (0xffffa02f));
    g.setGradientFill (shell);
    g.fillEllipse (faceCX - faceR * 1.03f, faceCY - faceR * 1.03f,
                   faceR * 2.06f, faceR * 2.06f);

    // Orange shell specular strokes, not a perfectly flat circle.
    juce::Path shellGlint;
    shellGlint.startNewSubPath (faceCX - faceR * 0.73f, faceCY - faceR * 0.78f);
    shellGlint.cubicTo (faceCX - faceR * 0.40f, faceCY - faceR * 1.01f,
                        faceCX + faceR * 0.18f, faceCY - faceR * 0.99f,
                        faceCX + faceR * 0.47f, faceCY - faceR * 0.86f);
    strokeRoundedPath (g, shellGlint, juce::Colour (0xffffd078).withAlpha (0.76f), 2.0f);

    juce::ColourGradient faceGradient (juce::Colour (0xffe8f8eb), faceCX - faceR * 0.50f,
                                       faceCY - faceR * 0.76f,
                                       juce::Colour (0xffc9e4d2), faceCX + faceR * 0.56f,
                                       faceCY + faceR * 0.86f, false);
    faceGradient.addColour (0.38, juce::Colour (0xffd9f0df));
    g.setGradientFill (faceGradient);
    g.fillEllipse (faceCX - faceR * 0.88f, faceCY - faceR * 0.88f,
                   faceR * 1.76f, faceR * 1.76f);

    // A faint forehead highlight makes the face enamel appear rounded.
    juce::Path foreheadGlint;
    foreheadGlint.startNewSubPath (faceCX - faceR * 0.53f, faceCY - faceR * 0.70f);
    foreheadGlint.quadraticTo (faceCX, faceCY - faceR * 0.88f,
                               faceCX + faceR * 0.45f, faceCY - faceR * 0.70f);
    strokeRoundedPath (g, foreheadGlint, juce::Colours::white.withAlpha (0.20f), 2.0f);

    // Brows and eyes: the expression can become more animated as the effect rises.
    const float eyeW = faceR * (0.175f + 0.012f * t);
    const float eyeH = faceR * (0.285f + 0.025f * t);
    const float eyeY = faceCY - faceR * 0.34f;
    const float eyeOffset = faceR * 0.43f;
    for (int side = -1; side <= 1; side += 2)
    {
        const float ex = faceCX + (float) side * eyeOffset - eyeW * 0.5f;
        juce::Path brow;
        brow.startNewSubPath (ex - eyeW * 0.02f, eyeY - faceR * 0.08f);
        brow.quadraticTo (ex + eyeW * 0.50f, eyeY - faceR * (0.15f + 0.025f * t),
                          ex + eyeW * 1.02f, eyeY - faceR * 0.06f);
        strokeRoundedPath (g, brow, juce::Colour (0xffb84a6f).withAlpha (0.48f), 2.0f);
        g.setColour (juce::Colour (0xff7c2447).withAlpha (0.55f));
        g.fillEllipse (ex - 1.0f, eyeY + 1.0f, eyeW + 2.0f, eyeH + 1.0f);
        g.setColour (juce::Colour (0xffa42f59));
        g.fillEllipse (ex, eyeY, eyeW, eyeH);
        g.setColour (juce::Colour (0xffffd1df).withAlpha (0.82f));
        g.fillEllipse (ex + eyeW * 0.20f, eyeY + eyeH * 0.12f,
                       eyeW * 0.22f, eyeH * 0.17f);
        g.setColour (juce::Colours::white.withAlpha (0.30f));
        g.fillEllipse (ex + eyeW * 0.62f, eyeY + eyeH * 0.58f,
                       eyeW * 0.09f, eyeH * 0.08f);
    }

    // Soft cheeks with two highlights give the face depth without muddy outlines.
    const float cheekAlpha = 0.08f + 0.13f * t;
    g.setColour (juce::Colour (0xffff8bae).withAlpha (cheekAlpha));
    g.fillEllipse (faceCX - faceR * 0.73f, faceCY + faceR * 0.13f,
                   faceR * 0.31f, faceR * 0.16f);
    g.fillEllipse (faceCX + faceR * 0.42f, faceCY + faceR * 0.13f,
                   faceR * 0.31f, faceR * 0.16f);
    g.setColour (juce::Colours::white.withAlpha (0.18f));
    g.drawLine (faceCX - faceR * 0.67f, faceCY + faceR * 0.18f,
                faceCX - faceR * 0.49f, faceCY + faceR * 0.18f, 1.0f);
    g.drawLine (faceCX + faceR * 0.49f, faceCY + faceR * 0.18f,
                faceCX + faceR * 0.67f, faceCY + faceR * 0.18f, 1.0f);

    // Tall glossy capsule nose, intentionally narrower than the eye spacing.
    const float noseTop = faceCY - faceR * 0.28f;
    const float noseBottom = faceCY + faceR * 0.085f;
    const float noseHalfWidth = faceR * (0.105f + 0.012f * t);
    juce::Path nose;
    nose.startNewSubPath (faceCX - noseHalfWidth * 0.52f, noseTop + faceR * 0.015f);
    nose.cubicTo (faceCX - noseHalfWidth * 1.06f, noseTop + faceR * 0.10f,
                  faceCX - noseHalfWidth * 0.96f, noseBottom - faceR * 0.03f,
                  faceCX - noseHalfWidth * 0.26f, noseBottom);
    nose.cubicTo (faceCX + noseHalfWidth * 0.26f, noseBottom + faceR * 0.015f,
                  faceCX + noseHalfWidth * 0.96f, noseBottom - faceR * 0.08f,
                  faceCX + noseHalfWidth * 0.52f, noseTop + faceR * 0.015f);
    nose.closeSubPath();
    juce::ColourGradient noseGradient (juce::Colour (0xffff9ac8), faceCX - noseHalfWidth, noseTop,
                                       juce::Colour (0xffa92e65), faceCX + noseHalfWidth, noseBottom, false);
    noseGradient.addColour (0.40, juce::Colour (0xffe45d9b));
    g.setGradientFill (noseGradient);
    g.fillPath (nose);
    g.setColour (juce::Colour (0xffffd0e2).withAlpha (0.80f));
    g.drawLine (faceCX - noseHalfWidth * 0.31f, noseTop + faceR * 0.06f,
                faceCX - noseHalfWidth * 0.22f, noseBottom - faceR * 0.10f, 1.5f);
    g.setColour (juce::Colour (0xff842746).withAlpha (0.60f));
    g.fillEllipse (faceCX - noseHalfWidth * 0.42f, noseBottom - faceR * 0.047f,
                   noseHalfWidth * 0.25f, faceR * 0.020f);
    g.fillEllipse (faceCX + noseHalfWidth * 0.16f, noseBottom - faceR * 0.047f,
                   noseHalfWidth * 0.25f, faceR * 0.020f);

    const float mouthHalfWidth = faceR * (0.57f + 0.19f * t);
    const float mouthTopY = faceCY + faceR * (0.205f + 0.012f * t);
    const float mouthDepth = faceR * (0.09f + 0.35f * t);

    if (t < 0.30f)
    {
        // Gentle smile at low intensity.
        juce::Path smileShadow;
        smileShadow.startNewSubPath (faceCX - mouthHalfWidth, mouthTopY + 2.0f);
        smileShadow.cubicTo (faceCX - mouthHalfWidth * 0.42f, mouthTopY + mouthDepth * 1.85f,
                             faceCX + mouthHalfWidth * 0.42f, mouthTopY + mouthDepth * 1.85f,
                             faceCX + mouthHalfWidth, mouthTopY + 2.0f);
        strokeRoundedPath (g, smileShadow, juce::Colour (0xff812746), faceR * 0.082f);
        juce::Path smile;
        smile.startNewSubPath (faceCX - mouthHalfWidth, mouthTopY);
        smile.cubicTo (faceCX - mouthHalfWidth * 0.42f, mouthTopY + mouthDepth * 1.75f,
                       faceCX + mouthHalfWidth * 0.42f, mouthTopY + mouthDepth * 1.75f,
                       faceCX + mouthHalfWidth, mouthTopY);
        strokeRoundedPath (g, smile, pink, faceR * 0.058f);
        strokeRoundedPath (g, smile, juce::Colour (0xffffb7d2).withAlpha (0.75f), 1.2f);
    }
    else
    {
        // Crescent mouth with a rounded chin curve and pink upper lip.
        const float topCurveDepth = faceR * (0.15f + 0.075f * t);
        const float bottomY = mouthTopY + mouthDepth;
        juce::Path mouth;
        mouth.startNewSubPath (faceCX - mouthHalfWidth, mouthTopY);
        mouth.cubicTo (faceCX - mouthHalfWidth * 0.54f, mouthTopY + topCurveDepth,
                       faceCX + mouthHalfWidth * 0.54f, mouthTopY + topCurveDepth,
                       faceCX + mouthHalfWidth, mouthTopY);
        mouth.cubicTo (faceCX + mouthHalfWidth * 0.84f, bottomY - mouthDepth * 0.08f,
                       faceCX + mouthHalfWidth * 0.43f, bottomY,
                       faceCX, bottomY);
        mouth.cubicTo (faceCX - mouthHalfWidth * 0.43f, bottomY,
                       faceCX - mouthHalfWidth * 0.84f, bottomY - mouthDepth * 0.08f,
                       faceCX - mouthHalfWidth, mouthTopY);
        mouth.closeSubPath();
        g.setColour (juce::Colour (0xff76233f));
        g.fillPath (mouth);

        // Interior burgundy gradient provides a little depth to the open grin.
        juce::ColourGradient mouthGradient (juce::Colour (0xffa5365b), faceCX, mouthTopY + topCurveDepth,
                                            juce::Colour (0xff671d3c), faceCX, bottomY, false);
        g.setGradientFill (mouthGradient);
        juce::Path innerMouth;
        innerMouth.startNewSubPath (faceCX - mouthHalfWidth * 0.87f, mouthTopY + topCurveDepth * 0.68f);
        innerMouth.cubicTo (faceCX - mouthHalfWidth * 0.49f, mouthTopY + topCurveDepth * 1.20f,
                            faceCX + mouthHalfWidth * 0.49f, mouthTopY + topCurveDepth * 1.20f,
                            faceCX + mouthHalfWidth * 0.87f, mouthTopY + topCurveDepth * 0.68f);
        innerMouth.cubicTo (faceCX + mouthHalfWidth * 0.66f, bottomY - mouthDepth * 0.14f,
                            faceCX - mouthHalfWidth * 0.66f, bottomY - mouthDepth * 0.14f,
                            faceCX - mouthHalfWidth * 0.87f, mouthTopY + topCurveDepth * 0.68f);
        innerMouth.closeSubPath();
        g.fillPath (innerMouth);

        juce::Path upperLip;
        upperLip.startNewSubPath (faceCX - mouthHalfWidth, mouthTopY);
        upperLip.cubicTo (faceCX - mouthHalfWidth * 0.54f, mouthTopY + topCurveDepth,
                          faceCX + mouthHalfWidth * 0.54f, mouthTopY + topCurveDepth,
                          faceCX + mouthHalfWidth, mouthTopY);
        strokeRoundedPath (g, upperLip, pink, faceR * 0.060f);
        strokeRoundedPath (g, upperLip, juce::Colour (0xffffb6d2).withAlpha (0.76f), 1.3f);

        // Six connected-looking upper teeth follow the smile arc, not a flat line.
        const int upperToothCount = 6;
        const float upperToothW = mouthHalfWidth * 0.255f;
        const float upperToothH = juce::jmax (5.0f, faceR * (0.070f + 0.012f * t));
        for (int tooth = 0; tooth < upperToothCount; ++tooth)
        {
            const float nx = ((float) tooth - ((float) upperToothCount - 1.0f) * 0.5f) / (((float) upperToothCount - 1.0f) * 0.5f);
            const float tx = faceCX + nx * mouthHalfWidth * 0.70f;
            const float curve = 1.0f - nx * nx;
            const float ty = mouthTopY + topCurveDepth * (0.72f * curve + 0.08f) + 1.5f;
            const float toothW = upperToothW * (1.0f - 0.08f * std::abs (nx));
            g.setColour (juce::Colour (0xff52182e).withAlpha (0.42f));
            g.fillRoundedRectangle (tx - toothW * 0.5f + 0.5f, ty + 1.0f,
                                    toothW, upperToothH, 2.5f);
            g.setColour (juce::Colour (0xffffe9e0));
            g.fillRoundedRectangle (tx - toothW * 0.5f, ty,
                                    toothW, upperToothH, 2.5f);
            g.setColour (juce::Colours::white.withAlpha (0.65f));
            g.drawLine (tx - toothW * 0.26f, ty + 1.1f,
                        tx + toothW * 0.19f, ty + 1.1f, 0.7f);
        }

        // Four small lower teeth sit near the corners so the tongue remains readable.
        const float lowerToothW = mouthHalfWidth * 0.14f;
        const float lowerToothH = juce::jmax (3.5f, faceR * 0.043f);
        for (int tooth = 0; tooth < 4; ++tooth)
        {
            const float side = tooth < 2 ? -1.0f : 1.0f;
            const float position = (tooth % 2 == 0) ? 0.70f : 0.48f;
            const float tx = faceCX + side * mouthHalfWidth * position;
            const float ty = mouthTopY + mouthDepth * (0.78f - 0.025f * position);
            g.setColour (juce::Colour (0xff4c172d).withAlpha (0.40f));
            g.fillRoundedRectangle (tx - lowerToothW * 0.5f + 0.5f, ty + 0.7f,
                                    lowerToothW, lowerToothH, 2.0f);
            g.setColour (juce::Colour (0xffffe8df));
            g.fillRoundedRectangle (tx - lowerToothW * 0.5f, ty,
                                    lowerToothW, lowerToothH, 2.0f);
        }

        if (t > 0.62f)
        {
            g.setColour (juce::Colour (0xffee6699));
            g.fillRoundedRectangle (faceCX - mouthHalfWidth * 0.25f,
                                    mouthTopY + mouthDepth * 0.72f,
                                    mouthHalfWidth * 0.50f, faceR * 0.085f, faceR * 0.04f);
            g.setColour (juce::Colour (0xffa73760).withAlpha (0.72f));
            g.drawLine (faceCX, mouthTopY + mouthDepth * 0.75f,
                        faceCX, mouthTopY + mouthDepth * 0.82f, 0.9f);
        }
    }

    // Tiny footer remains clear of the character shoes.
    g.setColour (juce::Colour (0xffddd8d0).withAlpha (0.86f));
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
