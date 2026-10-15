#include "PluginEditor.h"
#include <cmath>

namespace
{

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
    const float side = (float) juce::jmin (width, height);
    const float cx = (float) x + (float) width * 0.5f;
    // Keep the dial centre tied to its width so added readout space below
    // the knob does not push the visual dial down as the editor is resized.
    const float cy = (float) y + (float) width * 0.60f;
    const float radius = side * 0.455f;
    // JUCE angles run clockwise from 12 o'clock, which is what Path::addCentredArc
    // expects. Direct cos/sin placement needs the quarter-turn offset below.
    const float start = rotaryStartAngle;
    const float end = rotaryEndAngle;
    const float quarterTurn = juce::MathConstants<float>::halfPi;
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
        const float x1 = cx + std::cos (a - quarterTurn) * inner;
        const float y1 = cy + std::sin (a - quarterTurn) * inner;
        const float x2 = cx + std::cos (a - quarterTurn) * outer;
        const float y2 = cy + std::sin (a - quarterTurn) * outer;
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

    const float markerX = cx + std::cos (current - quarterTurn) * radius;
    const float markerY = cy + std::sin (current - quarterTurn) * radius;
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

    juce::ColourGradient body (juce::Colour (0xffe4e5e7), cx - knobR * 0.48f, cy - knobR * 0.70f,
                               juce::Colour (0xff6f7175), cx + knobR * 0.78f, cy + knobR * 0.78f, true);
    body.addColour (0.38, juce::Colour (0xffc6c7ca));
    body.addColour (0.72, juce::Colour (0xff97999d));
    g.setGradientFill (body);
    g.fillEllipse (cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f);

    // Fine concentric rings give the disc a spun, brushed-metal finish.
    for (float r = 2.5f; r < knobR - 1.0f; r += 2.2f)
    {
        const bool light = ((int) (r / 2.2f)) % 2 == 0;
        g.setColour ((light ? juce::Colours::white : juce::Colours::black).withAlpha (light ? 0.07f : 0.05f));
        g.drawEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f, 0.7f);
    }

    // Subtle glints on the upper edge of the metal ring.
    juce::Path bezelGlint;
    bezelGlint.addCentredArc (cx, cy, knobR + 1.0f, knobR + 1.0f, 0.0f,
                              juce::MathConstants<float>::pi * 1.08f,
                              juce::MathConstants<float>::pi * 1.88f, true);
    strokeRoundedPath (g, bezelGlint, juce::Colours::white.withAlpha (0.28f), 1.4f);

    // A chunky, angled grip makes the control read as a physical plugin knob,
    // rather than a flat pointer. Its angle tracks the real attached parameter.
    const float pointerAngle = current;
    juce::Path grip;
    // Long tapered blade: broad rounded tail, widest near the middle, narrowing
    // to a pointed tip on the +x end (which points at the current value).
    grip.startNewSubPath (cx - knobR * 0.95f, cy - knobR * 0.26f);
    grip.quadraticTo (cx - knobR * 0.20f, cy - knobR * 0.72f,
                      cx + knobR * 0.55f, cy - knobR * 0.20f);
    grip.lineTo (cx + knobR * 0.98f, cy + knobR * 0.02f);
    grip.lineTo (cx + knobR * 0.50f, cy + knobR * 0.28f);
    grip.quadraticTo (cx - knobR * 0.25f, cy + knobR * 0.64f,
                      cx - knobR * 0.95f, cy + knobR * 0.22f);
    grip.closeSubPath();
    grip.applyTransform (juce::AffineTransform::rotation (pointerAngle - quarterTurn, cx, cy));
    g.setColour (juce::Colours::black.withAlpha (0.62f));
    g.fillPath (grip, juce::AffineTransform::translation (1.8f, 2.4f));
    juce::ColourGradient gripGradient (juce::Colour (0xff3a3c40), cx - knobR * 0.3f, cy - knobR * 0.35f,
                                       juce::Colour (0xff050607), cx + knobR * 0.22f, cy + knobR * 0.45f, false);
    gripGradient.addColour (0.28, juce::Colour (0xff242629));
    gripGradient.addColour (0.51, juce::Colour (0xff111315));
    g.setGradientFill (gripGradient);
    g.fillPath (grip);
    g.setColour (juce::Colour (0xff6b6c70).withAlpha (0.88f));
    g.strokePath (grip, juce::PathStrokeType (0.8f));

    // Light tip facet so the pointing end is unmistakable.
    juce::Path gripTip;
    gripTip.startNewSubPath (cx + knobR * 0.98f, cy + knobR * 0.02f);
    gripTip.lineTo (cx + knobR * 0.72f, cy - knobR * 0.10f);
    gripTip.lineTo (cx + knobR * 0.74f, cy + knobR * 0.14f);
    gripTip.closeSubPath();
    gripTip.applyTransform (juce::AffineTransform::rotation (pointerAngle - quarterTurn, cx, cy));
    g.setColour (juce::Colour (0xffdadbdd).withAlpha (0.92f));
    g.fillPath (gripTip);

    // Curved specular ridge along the blade.
    juce::Path gripGlint;
    gripGlint.startNewSubPath (cx - knobR * 0.72f, cy - knobR * 0.10f);
    gripGlint.quadraticTo (cx - knobR * 0.10f, cy - knobR * 0.32f,
                           cx + knobR * 0.58f, cy - knobR * 0.06f);
    gripGlint.applyTransform (juce::AffineTransform::rotation (pointerAngle - quarterTurn, cx, cy));
    strokeRoundedPath (g, gripGlint, juce::Colours::white.withAlpha (0.50f), 2.2f);
    g.setColour (juce::Colour (0xffa5a8ab).withAlpha (0.92f));
    g.fillEllipse (cx - 2.8f, cy - 2.8f, 5.6f, 5.6f);

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
    juce::ignoreUnused (isButtonDown, buttonX, buttonY, buttonW, buttonH);
    const int selected = box.getSelectedId();
    const juce::String category = selected <= 2 ? "SOFT"
        : (selected <= 4 ? "MEDIUM" : (selected <= 6 ? "HARD" : "EXTREME"));
    const juce::Colour tabColour = selected <= 2 ? juce::Colour (0xffffa13b)
        : (selected <= 4 ? juce::Colour (0xff46b8d5)
        : (selected <= 6 ? juce::Colour (0xfff04491) : juce::Colour (0xffd84986)));

    // Cast-metal pink housing and lower shadow.
    g.setColour (juce::Colours::black.withAlpha (0.34f));
    g.fillRoundedRectangle (3.0f, 4.0f, (float) width - 6.0f, (float) height - 5.0f, 5.0f);
    juce::ColourGradient casing (juce::Colour (0xffff79aa), 0.0f, 0.0f,
                                  juce::Colour (0xffbd3c70), (float) width, (float) height, false);
    casing.addColour (0.42, juce::Colour (0xffed4c8e));
    g.setGradientFill (casing);
    g.fillRoundedRectangle (1.0f, 0.5f, (float) width - 4.0f, (float) height - 7.0f, 5.5f);
    g.setColour (juce::Colour (0xffffb2cd).withAlpha (0.82f));
    g.drawRoundedRectangle (1.5f, 1.0f, (float) width - 5.0f, (float) height - 8.0f, 5.0f, 1.0f);
    g.setColour (juce::Colour (0xff8c244e).withAlpha (0.85f));
    g.drawRoundedRectangle (4.0f, 3.5f, (float) width - 10.0f, (float) height - 13.0f, 3.0f, 1.0f);

    // Cream paper window, gently inset into the casing.
    juce::ColourGradient paper (juce::Colour (0xfffffdf1), 8.0f, 21.0f,
                                 juce::Colour (0xffe3ddd2), (float) width - 12.0f, (float) height, false);
    paper.addColour (0.52, juce::Colour (0xfff5f0e7));
    g.setGradientFill (paper);
    g.fillRoundedRectangle (9.0f, 22.0f, (float) width - 20.0f, (float) height - 31.0f, 3.0f);
    g.setColour (juce::Colour (0xff9c8e8c).withAlpha (0.80f));
    g.drawRoundedRectangle (9.0f, 22.0f, (float) width - 20.0f, (float) height - 31.0f, 3.0f, 0.8f);

    // The category is a separate tab, like the original labelled preset cassette.
    const float tabWidth = category == "EXTREME" ? 79.0f : (category == "MEDIUM" ? 72.0f : 56.0f);
    const float tabX = (float) width - tabWidth - 22.0f;
    g.setColour (juce::Colour (0xff6e1c3c).withAlpha (0.55f));
    g.fillRoundedRectangle (tabX + 1.5f, 4.5f, tabWidth + 2.0f, 16.5f, 1.5f);
    g.setColour (juce::Colour (0xff2b2b2e));
    g.fillRoundedRectangle (tabX, 3.0f, tabWidth, 16.0f, 1.5f);
    g.setColour (tabColour);
    g.fillRect (tabX + 3.0f, 16.0f, tabWidth - 6.0f, 2.0f);
    g.setColour (juce::Colour (0xfff1ece6));
    g.setFont (juce::Font (juce::FontOptions (10.5f, juce::Font::bold)));
    g.drawText (category, juce::Rectangle<int> ((int) tabX + 2, 3, (int) tabWidth - 4, 13),
                juce::Justification::centred);

    // Rivets and an understated down-chevron remain clear at small sizes.
    for (const float sx : { 7.0f, (float) width - 9.0f })
    {
        g.setColour (juce::Colour (0xff8a244a));
        g.fillEllipse (sx - 1.7f, (float) height * 0.53f - 1.7f, 3.4f, 3.4f);
        g.setColour (juce::Colour (0xffffd1e1));
        g.fillEllipse (sx - 0.7f, (float) height * 0.53f - 1.1f, 1.3f, 1.3f);
    }
    const float arrowCX = (float) width - 25.0f;
    const float arrowCY = (float) height * 0.66f;
    juce::Path arrow;
    arrow.startNewSubPath (arrowCX - 6.0f, arrowCY - 2.0f);
    arrow.lineTo (arrowCX, arrowCY + 3.5f);
    arrow.lineTo (arrowCX + 6.0f, arrowCY - 2.0f);
    strokeRoundedPath (g, arrow, juce::Colour (0xff342630), 2.4f);
}

void EndlessSmileInspiredLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (18, 25, juce::jmax (1, box.getWidth() - 58), juce::jmax (1, box.getHeight() - 34));
    label.setJustificationType (juce::Justification::centredLeft);
    label.setFont (juce::Font (juce::FontOptions (15.0f, juce::Font::italic)));
}

void EndlessSmileInspiredLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
    const juce::Colour& backgroundColour, bool highlighted, bool down)
{
    juce::ignoreUnused (backgroundColour);
    const auto b = button.getLocalBounds().toFloat();
    const bool previous = button.getName() == "preset-previous";
    const float midY = b.getCentreY();
    const float inset = 5.0f;
    juce::Path arrow;
    if (previous)
    {
        arrow.startNewSubPath (inset, midY);
        arrow.lineTo (b.getWidth() * 0.44f, midY - b.getHeight() * 0.43f);
        arrow.lineTo (b.getWidth() * 0.44f, midY - b.getHeight() * 0.20f);
        arrow.lineTo (b.getWidth() - inset, midY - b.getHeight() * 0.20f);
        arrow.lineTo (b.getWidth() - inset, midY + b.getHeight() * 0.20f);
        arrow.lineTo (b.getWidth() * 0.44f, midY + b.getHeight() * 0.20f);
        arrow.lineTo (b.getWidth() * 0.44f, midY + b.getHeight() * 0.43f);
    }
    else
    {
        arrow.startNewSubPath (b.getWidth() - inset, midY);
        arrow.lineTo (b.getWidth() * 0.56f, midY - b.getHeight() * 0.43f);
        arrow.lineTo (b.getWidth() * 0.56f, midY - b.getHeight() * 0.20f);
        arrow.lineTo (inset, midY - b.getHeight() * 0.20f);
        arrow.lineTo (inset, midY + b.getHeight() * 0.20f);
        arrow.lineTo (b.getWidth() * 0.56f, midY + b.getHeight() * 0.20f);
        arrow.lineTo (b.getWidth() * 0.56f, midY + b.getHeight() * 0.43f);
    }
    arrow.closeSubPath();
    g.setColour (juce::Colours::black.withAlpha (0.32f));
    g.fillPath (arrow, juce::AffineTransform::translation (1.0f, 2.0f));
    const juce::Colour arrowTop = down ? juce::Colour (0xffe8789f)
                                       : (highlighted ? juce::Colour (0xffffcb78) : juce::Colour (0xffffb04d));
    const juce::Colour arrowBottom = down ? juce::Colour (0xffc94577)
                                          : (highlighted ? juce::Colour (0xffff9a2a) : juce::Colour (0xfff0780f));
    g.setGradientFill (juce::ColourGradient (arrowTop, 0.0f, b.getY(),
                                             arrowBottom, 0.0f, b.getBottom(), false));
    g.fillPath (arrow);
    g.setColour (juce::Colour (0xff9b4c0c).withAlpha (0.85f));
    g.strokePath (arrow, juce::PathStrokeType (1.4f, juce::PathStrokeType::mitered));
}

void EndlessSmileInspiredLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
    bool isMouseOverButton, bool isButtonDown)
{
    juce::ignoreUnused (g, button, isMouseOverButton, isButtonDown);
    // The buttons intentionally use custom arrow silhouettes instead of text glyphs.
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
    intensity.setRotaryParameters (juce::MathConstants<float>::pi * 1.20f,
                                  juce::MathConstants<float>::pi * 2.80f, true);
    intensity.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    intensity.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (intensity);

    // ASCII separators avoid mojibake in MSVC builds with inconsistent source encodings.
    const juce::StringArray presetNames {
        "FIST IN THE AIR",
        "HELIUM JUMPING",
        "NO LIMIT",
        "FACE MELTING JOY",
        "SMACK YOU IN THE FACE",
        "BROKEN TEETH",
        "DADA DYNAMITE"
    };
    for (int i = 0; i < presetNames.size(); ++i)
        preset.addItem (presetNames[i], i + 1);
    preset.setJustificationType (juce::Justification::centredLeft);
    preset.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xffed4c8e));
    preset.setColour (juce::ComboBox::textColourId, juce::Colour (0xff30252b));
    preset.setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff9f2e59));
    preset.setColour (juce::ComboBox::arrowColourId, juce::Colour (0xff30252b));
    preset.setTooltip ("Choose a build-up profile. Previous/next arrows also cycle the seven presets.");
    preset.setWantsKeyboardFocus (true);
    preset.onChange = [this]
    {
        const int id = preset.getSelectedId();
        switch (id)
        {
            case 1: preset.setTooltip ("Fist in the Air - Soft: reverb and a gentle high-pass sweep."); break;
            case 2: preset.setTooltip ("Helium Jumping - Soft: reverb, high-pass sweep and delay."); break;
            case 3: preset.setTooltip ("No Limit - Medium: reverb and a full high-pass sweep."); break;
            case 4: preset.setTooltip ("Face Melting Joy - Medium: reverb, high-pass sweep and delay."); break;
            case 5: preset.setTooltip ("Smack You in the Face - Hard: reverb, full high-pass, noise and riser."); break;
            case 6: preset.setTooltip ("Broken Teeth - Hard: all hard effects plus delay."); break;
            case 7: preset.setTooltip ("Dada Dynamite - Extreme: all effects; noise and riser follow intensity."); break;
            default: preset.setTooltip ("Choose one of the seven build-up profiles."); break;
        }
    };
    addAndMakeVisible (preset);

    previousPresetButton.setName ("preset-previous");
    previousPresetButton.setButtonText ("");
    previousPresetButton.setTooltip ("Previous preset");
    previousPresetButton.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    previousPresetButton.onClick = [this]
    {
        const int id = preset.getSelectedId();
        preset.setSelectedId (id <= 1 ? 7 : id - 1, juce::sendNotificationSync);
    };
    addAndMakeVisible (previousPresetButton);

    nextPresetButton.setName ("preset-next");
    nextPresetButton.setButtonText ("");
    nextPresetButton.setTooltip ("Next preset");
    nextPresetButton.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    nextPresetButton.onClick = [this]
    {
        const int id = preset.getSelectedId();
        preset.setSelectedId (id >= 7 ? 1 : id + 1, juce::sendNotificationSync);
    };
    addAndMakeVisible (nextPresetButton);

    intensityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (p.state, "intensity", intensity);
    presetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        (p.state, "preset", preset);

    // Begin from the restored parameter state instead of animating from zero
    // whenever the editor is reopened.
    animatedIntensity = juce::jlimit (0.0f, 1.0f, (float) intensity.getValue() / 110.0f);
    preset.onChange();
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
    // Use the smoothed intensity for the mascot only. The actual rotary control
    // and numeric readout remain directly attached to the true parameter value.
    const float t = juce::jlimit (0.0f, 1.0f, animatedIntensity);
    const float ease = t * t * (3.0f - 2.0f * t);

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
    const float faceR = juce::jmin (w * 0.134f, h * 0.228f);
    const float bounceX = std::sin (animationPhase * 0.83f) * faceR * 0.010f * ease;
    const float bounceY = std::cos (animationPhase * 1.17f) * faceR * 0.012f * ease;
    const float faceCX = w * 0.758f + bounceX;
    const float faceCY = h * juce::jmap (ease, 0.535f, 0.515f) + bounceY;
    const float torsoTop = faceCY + faceR * (0.70f - 0.025f * ease);
    const float torsoBottom = h * (0.855f + 0.025f * ease);
    const float legBottom = h * (0.905f + 0.014f * ease);
    const float shoeBottom = legBottom + h * (0.032f + 0.004f * ease);

    // A tiny intensity-weighted squash/stretch gives the vector mascot some
    // physical spring without changing the host's control positions.
    const float squashPulse = std::sin (animationPhase * 2.0f) * 0.012f * ease;
    const float characterScaleX = 1.0f + 0.014f * ease + squashPulse;
    const float characterScaleY = 1.0f - 0.008f * ease - squashPulse * 0.75f;
    g.saveState();
    g.addTransform (juce::AffineTransform::scale (characterScaleX, characterScaleY, faceCX, faceCY));

    // ---- Skeleton character ------------------------------------------------
    // Original bone-and-skull mascot. The same intensity morph drives it: slumped,
    // tilted skull with a frown and drooping arms at zero; raised arms, wide-open
    // sockets and a big toothy grin at maximum.
    const juce::Colour boneLight (0xffeee6d2);
    const juce::Colour boneShade (0xffbfb49a);
    const juce::Colour boneEdge (0xff6b6150);
    const juce::Colour socketDark (0xff1b181c);

    const auto drawBone = [&g, boneLight, boneEdge] (float x1, float y1, float x2, float y2, float thickness)
    {
        g.setColour (juce::Colours::black.withAlpha (0.25f));
        g.drawLine (x1 + 1.5f, y1 + 1.5f, x2 + 1.5f, y2 + 1.5f, thickness + 3.0f);
        g.setColour (boneEdge);
        g.drawLine (x1, y1, x2, y2, thickness + 1.6f);
        g.setColour (boneLight);
        g.drawLine (x1, y1, x2, y2, thickness - 0.8f);
        const float r = thickness * 0.78f;
        for (const juce::Point<float> pt : { juce::Point<float> (x1, y1), juce::Point<float> (x2, y2) })
        {
            g.setColour (boneEdge);
            g.fillEllipse (pt.x - r - 0.8f, pt.y - r - 0.8f, (r + 0.8f) * 2.0f, (r + 0.8f) * 2.0f);
            g.setColour (boneLight);
            g.fillEllipse (pt.x - r, pt.y - r, r * 2.0f, r * 2.0f);
        }
    };

    const auto cubicAt = [] (float u, juce::Point<float> a, juce::Point<float> b,
                             juce::Point<float> c, juce::Point<float> d)
    {
        const float m = 1.0f - u;
        return a * (m * m * m) + b * (3.0f * m * m * u) + c * (3.0f * m * u * u) + d * (u * u * u);
    };

    // Ground shadow.
    g.setColour (juce::Colours::black.withAlpha (0.22f));
    g.fillEllipse (faceCX - faceR * 0.62f, shoeBottom - 4.0f, faceR * 1.24f, 9.0f);

    // Legs: thigh, shin and a flat foot bone.
    for (int side = -1; side <= 1; side += 2)
    {
        const float legX = faceCX + (float) side * faceR * 0.15f;
        const float footY = shoeBottom - 4.0f;
        drawBone (legX, torsoBottom - 6.0f, legX, legBottom, 7.0f);
        drawBone (legX, legBottom, legX, footY, 6.0f);
        drawBone (legX, footY, legX + (float) side * faceR * 0.26f, footY, 6.0f);
    }

    // Pelvis.
    const float pelvisY = torsoBottom - 14.0f;
    g.setColour (boneEdge);
    g.fillEllipse (faceCX - faceR * 0.34f - 1.0f, pelvisY - 1.0f, faceR * 0.68f + 2.0f, 18.0f);
    g.setColour (boneLight);
    g.fillEllipse (faceCX - faceR * 0.34f, pelvisY, faceR * 0.68f, 16.0f);
    g.setColour (socketDark);
    g.fillEllipse (faceCX - faceR * 0.075f, pelvisY + 4.0f, faceR * 0.15f, 8.0f);

    // Ribcage: dark cavity, four rib pairs, then the spine on top.
    const float cageTop = torsoTop - faceR * 0.10f;
    g.setColour (socketDark.withAlpha (0.62f));
    g.fillEllipse (faceCX - faceR * 0.46f, cageTop, faceR * 0.92f, (pelvisY + 2.0f) - cageTop);

    const float visibleTorsoTop = faceCY + faceR * 0.93f;
    for (int rib = 0; rib < 4; ++rib)
    {
        const float ribY = visibleTorsoTop + (pelvisY - visibleTorsoTop) * (0.08f + 0.22f * (float) rib);
        const float ribHalfWidth = faceR * (0.44f - 0.03f * (float) rib);
        for (int side = -1; side <= 1; side += 2)
        {
            const float sd = (float) side;
            juce::Path ribPath;
            ribPath.startNewSubPath (faceCX + sd * 3.0f, ribY);
            ribPath.cubicTo (faceCX + sd * ribHalfWidth * 0.60f, ribY - 4.0f,
                             faceCX + sd * ribHalfWidth, ribY,
                             faceCX + sd * ribHalfWidth * 0.92f, ribY + faceR * 0.10f);
            strokeRoundedPath (g, ribPath, boneEdge, 6.2f);
            strokeRoundedPath (g, ribPath, boneLight, 4.4f);
        }
    }
    for (float vy = torsoTop; vy < pelvisY; vy += 7.5f)
    {
        g.setColour (boneEdge);
        g.fillEllipse (faceCX - 4.4f, vy - 4.4f, 8.8f, 8.8f);
        g.setColour (boneLight);
        g.fillEllipse (faceCX - 3.6f, vy - 3.6f, 7.2f, 7.2f);
    }

    // Arms: the same intensity morph as before (drooped at zero, raised at maximum),
    // drawn as upper-arm and forearm bones with a splayed, four-finger hand.
    const float handY = juce::jmap (ease, faceCY + faceR * 0.90f, h * 0.075f);
    const float shoulderY = faceCY + faceR * (0.43f - 0.035f * ease);
    const float leftShoulderX = faceCX - faceR * 0.67f;
    const float rightShoulderX = faceCX + faceR * 0.67f;
    const float handSpread = 1.10f + 0.10f * ease;
    const float armSway = std::sin (animationPhase * 1.65f) * faceR * 0.025f * ease;
    const float leftHandX = faceCX - faceR * handSpread - armSway;
    const float rightHandX = faceCX + faceR * handSpread + armSway;
    const float armControlY1 = faceCY + faceR * (0.76f - 0.49f * ease);
    const float armControlY2 = handY + faceR * (-0.015f + 0.69f * ease);

    const auto drawArm = [&] (juce::Point<float> p0, juce::Point<float> p1,
                              juce::Point<float> p2, juce::Point<float> p3)
    {
        const juce::Point<float> elbow = cubicAt (0.5f, p0, p1, p2, p3);
        drawBone (p0.x, p0.y, elbow.x, elbow.y, 8.5f + 1.0f * ease);
        drawBone (elbow.x, elbow.y, p3.x, p3.y, 7.0f + 1.0f * ease);

        juce::Point<float> dir = p3 - elbow;
        const float len = std::sqrt (dir.x * dir.x + dir.y * dir.y);
        if (len > 0.001f)
            dir = dir / len;

        for (int k = 0; k < 4; ++k)
        {
            const float a = ((float) k - 1.5f) * 0.30f;
            const float c = std::cos (a);
            const float s = std::sin (a);
            const juce::Point<float> fd (dir.x * c - dir.y * s, dir.x * s + dir.y * c);
            const float fingerLength = faceR * ((k == 0 || k == 3) ? 0.115f : 0.135f);
            drawBone (p3.x, p3.y, p3.x + fd.x * fingerLength, p3.y + fd.y * fingerLength, 3.2f);
        }
    };

    drawArm ({ leftShoulderX, shoulderY },
             { faceCX - faceR * (0.88f + 0.11f * ease), armControlY1 },
             { leftHandX + faceR * 0.13f, armControlY2 },
             { leftHandX, handY });
    drawArm ({ rightShoulderX, shoulderY },
             { faceCX + faceR * (0.88f + 0.11f * ease), armControlY1 },
             { rightHandX - faceR * 0.13f, armControlY2 },
             { rightHandX, handY });

    // ---- Skull (tilted forward when miserable) -----------------------------
    const float headTilt = 0.075f * (1.0f - ease) + std::sin (animationPhase * 1.3f) * 0.018f * ease;
    g.saveState();
    g.addTransform (juce::AffineTransform::rotation (headTilt, faceCX, faceCY + faceR * 0.75f));

    g.setColour (juce::Colours::black.withAlpha (0.26f));
    g.fillEllipse (faceCX - faceR + 5.0f, faceCY - faceR + 8.0f, faceR * 2.0f, faceR * 2.0f);

    juce::ColourGradient skull (juce::Colour (0xfffbf6ea), faceCX - faceR * 0.45f, faceCY - faceR * 0.55f,
                                boneShade, faceCX + faceR * 0.85f, faceCY + faceR * 0.85f, true);
    g.setGradientFill (skull);
    g.fillEllipse (faceCX - faceR, faceCY - faceR, faceR * 2.0f, faceR * 2.0f);
    g.setColour (boneEdge);
    g.drawEllipse (faceCX - faceR, faceCY - faceR, faceR * 2.0f, faceR * 2.0f, 2.4f);

    // A small crack and cheekbone shading give the skull some character.
    juce::Path crack;
    crack.startNewSubPath (faceCX + faceR * 0.18f, faceCY - faceR * 0.985f);
    crack.lineTo (faceCX + faceR * 0.10f, faceCY - faceR * 0.86f);
    crack.lineTo (faceCX + faceR * 0.20f, faceCY - faceR * 0.76f);
    crack.lineTo (faceCX + faceR * 0.12f, faceCY - faceR * 0.64f);
    strokeRoundedPath (g, crack, boneEdge.withAlpha (0.70f), 1.6f);
    g.setColour (boneShade.withAlpha (0.28f));
    g.fillEllipse (faceCX - faceR * 0.80f, faceCY + faceR * 0.02f, faceR * 0.36f, faceR * 0.22f);
    g.fillEllipse (faceCX + faceR * 0.44f, faceCY + faceR * 0.02f, faceR * 0.36f, faceR * 0.22f);

    // Eye sockets: bigger and rounder as intensity rises; lids droop when miserable.
    const float eyeW = faceR * (0.16f + 0.09f * ease);
    const float eyeH = faceR * (0.155f + 0.300f * ease);
    const float eyeY = faceCY - faceR * (0.325f + 0.030f * ease);
    const float eyeOffset = faceR * 0.43f;
    const float lidAmount = juce::jmax (0.64f * (1.0f - ease),
                                        0.98f * blinkAmount * (1.0f - 0.85f * ease));
    for (int side = -1; side <= 1; side += 2)
    {
        const float sd = (float) side;
        const float eyeCX = faceCX + sd * eyeOffset;
        const float ex = eyeCX - eyeW * 0.5f;
        const float eyeCY = eyeY + eyeH * 0.5f;
        const float eyeTilt = sd * (0.04f + 0.14f * ease);

        juce::Path socket;
        socket.addEllipse (ex, eyeY, eyeW, eyeH);
        socket.applyTransform (juce::AffineTransform::rotation (eyeTilt, eyeCX, eyeCY));
        g.setColour (socketDark);
        g.fillPath (socket);

        // A small pale glint in each socket; it sits low when miserable.
        const float glintR = eyeW * 0.11f;
        const float glintY = eyeCY + eyeH * (0.10f * (1.0f - ease) - 0.06f * ease);
        g.setColour (juce::Colour (0xffe9fbff).withAlpha (0.92f));
        g.fillEllipse (eyeCX - glintR, glintY - glintR, glintR * 2.0f, glintR * 2.0f);

        if (lidAmount > 0.01f)
        {
            const float lidY = eyeY + eyeH * lidAmount;
            g.saveState();
            g.reduceClipRegion (socket, juce::AffineTransform());
            g.setColour (boneLight);
            g.fillRect (eyeCX - eyeW, eyeY - eyeH * 0.5f, eyeW * 2.0f, lidY - (eyeY - eyeH * 0.5f));
            g.setColour (boneEdge);
            g.drawLine (eyeCX - eyeW, lidY, eyeCX + eyeW, lidY, 1.6f);
            g.restoreState();
        }
        g.setColour (boneEdge);
        g.strokePath (socket, juce::PathStrokeType (1.6f));

        // Brow ridge: inner end raised when sad, outer end raised when happy.
        const float browY = eyeY - faceR * 0.06f;
        juce::Path brow;
        brow.startNewSubPath (eyeCX - sd * eyeW * 0.62f, browY - faceR * 0.07f * (1.0f - ease));
        brow.quadraticTo (eyeCX, browY - faceR * 0.05f,
                          eyeCX + sd * eyeW * 0.62f, browY - faceR * 0.07f * ease);
        strokeRoundedPath (g, brow, boneEdge.withAlpha (0.75f), 2.2f);
    }

    // Nasal cavity.
    juce::Path nose;
    nose.startNewSubPath (faceCX, faceCY - faceR * 0.02f);
    nose.quadraticTo (faceCX - faceR * 0.03f, faceCY + faceR * 0.05f,
                      faceCX - faceR * 0.085f, faceCY + faceR * 0.14f);
    nose.lineTo (faceCX, faceCY + faceR * 0.105f);
    nose.lineTo (faceCX + faceR * 0.085f, faceCY + faceR * 0.14f);
    nose.quadraticTo (faceCX + faceR * 0.03f, faceCY + faceR * 0.05f,
                      faceCX, faceCY - faceR * 0.02f);
    nose.closeSubPath();
    g.setColour (socketDark);
    g.fillPath (nose);

    // Mouth: a stitched frown at zero, turning to a smile and then a wide toothy grin.
    const float mouthHalfWidth = faceR * (0.43f + 0.35f * ease);
    const float mouthTopY = faceCY + faceR * (0.205f + 0.012f * t);
    const float openT = juce::jlimit (0.0f, 1.0f, (t - 0.18f) / 0.82f);
    const float mouthDepth = faceR * (0.04f + 0.44f * openT);

    if (t < 0.30f)
    {
        const float frownAmount = 1.0f - juce::jlimit (0.0f, 1.0f, t / 0.30f);
        const float cornerY = mouthTopY + faceR * 0.065f * frownAmount;
        const float centreY = mouthTopY - faceR * 0.042f * frownAmount
                            + faceR * 0.075f * (1.0f - frownAmount);
        const juce::Point<float> m0 (faceCX - mouthHalfWidth, cornerY);
        const juce::Point<float> m1 (faceCX - mouthHalfWidth * 0.40f, centreY);
        const juce::Point<float> m2 (faceCX + mouthHalfWidth * 0.40f, centreY);
        const juce::Point<float> m3 (faceCX + mouthHalfWidth, cornerY);
        juce::Path closedMouth;
        closedMouth.startNewSubPath (m0);
        closedMouth.cubicTo (m1, m2, m3);
        strokeRoundedPath (g, closedMouth, socketDark, faceR * 0.05f);
        for (int k = 1; k < 10; ++k)
        {
            const juce::Point<float> tp = cubicAt ((float) k / 10.0f, m0, m1, m2, m3);
            g.setColour (socketDark);
            g.drawLine (tp.x, tp.y - faceR * 0.035f, tp.x, tp.y + faceR * 0.035f, 1.4f);
        }
    }
    else
    {
        const float topCurveDepth = faceR * (0.045f + 0.17f * openT);
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
        g.setColour (socketDark);
        g.fillPath (mouth);
        g.setColour (boneEdge);
        g.strokePath (mouth, juce::PathStrokeType (1.8f));

        const auto drawTooth = [&g, boneLight, boneEdge] (float cxTooth, float topY, float widthTooth, float heightTooth)
        {
            g.setColour (boneLight);
            g.fillRoundedRectangle (cxTooth - widthTooth * 0.5f, topY, widthTooth, heightTooth, 2.0f);
            g.setColour (boneEdge);
            g.drawRoundedRectangle (cxTooth - widthTooth * 0.5f, topY, widthTooth, heightTooth, 2.0f, 0.9f);
        };

        // Upper teeth hang from the top edge (sized to stay inside the opening).
        const float upperEdgeSag = 0.75f * topCurveDepth;
        const float upperToothH = juce::jmax (2.0f, juce::jmin (faceR * (0.07f + 0.05f * openT),
                                                                mouthDepth - upperEdgeSag - 1.0f));
        const int upperCount = 8;
        const float upperToothW = mouthHalfWidth * 1.64f / (float) upperCount - 1.6f;
        for (int i = 0; i < upperCount; ++i)
        {
            const float u = -0.82f + 1.64f * ((float) i + 0.5f) / (float) upperCount;
            const float topY = mouthTopY + upperEdgeSag * (1.0f - u * u);
            drawTooth (faceCX + u * mouthHalfWidth, topY - 1.0f, upperToothW, upperToothH);
        }

        // Lower teeth appear once the jaw is open enough.
        if (mouthDepth > faceR * 0.26f)
        {
            const float lowerToothH = faceR * (0.06f + 0.045f * openT);
            const int lowerCount = 6;
            const float lowerToothW = mouthHalfWidth * 1.24f / (float) lowerCount - 1.6f;
            for (int i = 0; i < lowerCount; ++i)
            {
                const float u = -0.62f + 1.24f * ((float) i + 0.5f) / (float) lowerCount;
                const float edgeY = mouthTopY + mouthDepth * (1.0f - u * u * std::fabs (u));
                drawTooth (faceCX + u * mouthHalfWidth, edgeY - lowerToothH, lowerToothW, lowerToothH + 1.0f);
            }
        }
    }

    g.restoreState();  // end of tilted skull


    // Restore the panel coordinate system before drawing the fixed footer.
    g.restoreState();

    // Branded footer inspired by the original product-title strip while keeping
    // this clean-room build identified as the Inspired edition.
    g.setColour (juce::Colour (0xffe7e2dd).withAlpha (0.82f));
    g.setFont (juce::Font (juce::FontOptions (27.0f, juce::Font::bold)));
    g.drawText ("ENDLESS SMILE INSPIRED", 20, getHeight() - 57,
                juce::roundToInt (w * 0.56f), 34, juce::Justification::centredLeft,
                true);
    g.setColour (juce::Colour (0xffddd8d0).withAlpha (0.86f));
    g.setFont (juce::Font (juce::FontOptions (9.5f, juce::Font::bold)));
    g.drawText ("SKELLY AUDIO  |  BUILD-UP / TRANSITION EFFECT",
                20, getHeight() - 23, getWidth() - 40, 12, juce::Justification::centredLeft);
}

void EndlessSmileInspiredAudioProcessorEditor::resized()
{
    const auto bounds = getLocalBounds();
    const int dialWidth = juce::jmin (280, juce::roundToInt ((float) bounds.getHeight() * 0.40f));
    // The custom knob draws its readout and label below the ring; reserve enough
    // height for both so neither gets clipped at the default editor size.
    const int dialHeight = dialWidth + 115;
    const int dialX = juce::roundToInt ((float) bounds.getWidth() * 0.39f) - dialWidth / 2;
    const int dialY = juce::roundToInt ((float) bounds.getHeight() * 0.055f);
    intensity.setBounds (dialX, dialY, dialWidth, dialHeight);

    const int leftMargin = juce::roundToInt ((float) bounds.getWidth() * 0.044f);
    const int controlWidth = juce::jmin (680, juce::roundToInt ((float) bounds.getWidth() * 0.43f));
    const int presetY = juce::roundToInt ((float) bounds.getHeight() * 0.69f);
    preset.setBounds (leftMargin + 44, presetY + 1, controlWidth - 88, 68);
    previousPresetButton.setBounds (leftMargin - 18, presetY + 9, 62, 52);
    nextPresetButton.setBounds (leftMargin + controlWidth - 44, presetY + 9, 62, 52);
}

void EndlessSmileInspiredAudioProcessorEditor::timerCallback()
{
    // Smooth only the illustrated character, not the actual parameter. This
    // keeps the motion fluid under automation while the knob stays immediate.
    const float target = juce::jlimit (0.0f, 1.0f, (float) intensity.getValue() / 110.0f);
    animatedIntensity += (target - animatedIntensity) * 0.22f;
    if (std::abs (target - animatedIntensity) < 0.0005f)
        animatedIntensity = target;

    // Faster, more energetic motion at high intensity; nearly still at zero.
    animationPhase += 0.035f + 0.055f * animatedIntensity;
    if (animationPhase >= juce::MathConstants<float>::twoPi)
        animationPhase -= juce::MathConstants<float>::twoPi;

    ++animationFrame;
    if (framesUntilBlink <= 0)
    {
        blinkAmount = 1.0f;
        framesUntilBlink = 92 + (animationFrame % 61);
    }
    else
    {
        blinkAmount *= 0.55f;
        --framesUntilBlink;
    }

    repaint();
}
