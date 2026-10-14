#include "PluginEditor.h"
#include <cmath>

namespace
{
    const juce::Colour ink        (0xff1d2b30);
    const juce::Colour cyan       (0xff39afd2);
    const juce::Colour pink       (0xfff04491);

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
    grip.quadraticTo (cx - knobR * 0.20f, cy - knobR * 0.60f,
                      cx + knobR * 0.55f, cy - knobR * 0.20f);
    grip.lineTo (cx + knobR * 0.98f, cy + knobR * 0.02f);
    grip.lineTo (cx + knobR * 0.50f, cy + knobR * 0.28f);
    grip.quadraticTo (cx - knobR * 0.25f, cy + knobR * 0.52f,
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
    g.setColour (juce::Colour (0xff9b2d59).withAlpha (0.55f));
    g.fillRoundedRectangle (14.5f, 4.5f, tabWidth + 2.0f, 16.5f, 1.5f);
    g.setColour (tabColour);
    g.fillRoundedRectangle (13.0f, 3.0f, tabWidth, 16.0f, 1.5f);
    g.setColour (juce::Colour (0xff402432));
    g.setFont (juce::Font (juce::FontOptions (10.5f, juce::Font::bold)));
    g.drawText (category, juce::Rectangle<int> (15, 3, (int) tabWidth - 4, 16), juce::Justification::centred);

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
        arrow.lineTo (b.getWidth() * 0.44f, midY - b.getHeight() * 0.35f);
        arrow.lineTo (b.getWidth() * 0.44f, midY - b.getHeight() * 0.16f);
        arrow.lineTo (b.getWidth() - inset, midY - b.getHeight() * 0.16f);
        arrow.lineTo (b.getWidth() - inset, midY + b.getHeight() * 0.16f);
        arrow.lineTo (b.getWidth() * 0.44f, midY + b.getHeight() * 0.16f);
        arrow.lineTo (b.getWidth() * 0.44f, midY + b.getHeight() * 0.35f);
    }
    else
    {
        arrow.startNewSubPath (b.getWidth() - inset, midY);
        arrow.lineTo (b.getWidth() * 0.56f, midY - b.getHeight() * 0.35f);
        arrow.lineTo (b.getWidth() * 0.56f, midY - b.getHeight() * 0.16f);
        arrow.lineTo (inset, midY - b.getHeight() * 0.16f);
        arrow.lineTo (inset, midY + b.getHeight() * 0.16f);
        arrow.lineTo (b.getWidth() * 0.56f, midY + b.getHeight() * 0.16f);
        arrow.lineTo (b.getWidth() * 0.56f, midY + b.getHeight() * 0.35f);
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
    const float torsoBottom = h * (0.795f + 0.037f * ease);
    const float legBottom = h * (0.888f + 0.018f * ease);
    const float shoeBottom = legBottom + h * (0.032f + 0.004f * ease);

    // A tiny intensity-weighted squash/stretch gives the vector mascot some
    // physical spring without changing the host's control positions.
    const float squashPulse = std::sin (animationPhase * 2.0f) * 0.012f * ease;
    const float characterScaleX = 1.0f + 0.014f * ease + squashPulse;
    const float characterScaleY = 1.0f - 0.008f * ease - squashPulse * 0.75f;
    g.saveState();
    g.addTransform (juce::AffineTransform::scale (characterScaleX, characterScaleY, faceCX, faceCY));

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
    torso.startNewSubPath (faceCX - faceR * 0.39f, torsoTop);
    torso.cubicTo (faceCX - faceR * 0.50f, torsoTop + faceR * 0.18f,
                   faceCX - faceR * 0.43f, torsoBottom - faceR * 0.16f,
                   faceCX - faceR * 0.34f, torsoBottom);
    torso.cubicTo (faceCX - faceR * 0.14f, torsoBottom + 2.0f,
                   faceCX + faceR * 0.14f, torsoBottom + 2.0f,
                   faceCX + faceR * 0.34f, torsoBottom);
    torso.cubicTo (faceCX + faceR * 0.43f, torsoBottom - faceR * 0.16f,
                   faceCX + faceR * 0.50f, torsoTop + faceR * 0.18f,
                   faceCX + faceR * 0.39f, torsoTop);
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
    collar.startNewSubPath (faceCX - faceR * 0.39f, torsoTop + faceR * 0.025f);
    collar.quadraticTo (faceCX, torsoTop + faceR * 0.13f,
                        faceCX + faceR * 0.39f, torsoTop + faceR * 0.025f);
    strokeRoundedPath (g, collar, juce::Colour (0xff9b2d58), 5.0f);
    strokeRoundedPath (g, collar, juce::Colour (0xffff8fba).withAlpha (0.75f), 2.0f);

    for (int band = 0; band < 4; ++band)
    {
        const float u = 0.20f + (float) band * 0.19f;
        const float y = torsoTop + (torsoBottom - torsoTop) * u;
        const float halfW = faceR * (0.38f - (float) band * 0.020f);
        juce::Path seam;
        seam.startNewSubPath (faceCX - halfW, y);
        seam.cubicTo (faceCX - halfW * 0.42f, y + faceR * 0.065f,
                      faceCX + halfW * 0.42f, y + faceR * 0.065f,
                      faceCX + halfW, y);
        strokeRoundedPath (g, seam, juce::Colour (0xffa52859).withAlpha (0.76f), 1.5f);
        strokeRoundedPath (g, seam, juce::Colour (0xffff9fc2).withAlpha (0.56f), 0.8f);
    }

    // The arm pose is a real intensity morph: drooped beside the head at zero,
    // progressively arcing up, then nearly vertical at the maximum.
    const float handY = juce::jmap (ease, faceCY + faceR * 0.90f, h * 0.075f);
    const float shoulderY = faceCY + faceR * (0.43f - 0.035f * ease);
    const float leftShoulderX = faceCX - faceR * 0.67f;
    const float rightShoulderX = faceCX + faceR * 0.67f;
    // Keep hands just outside the head silhouette even in the slumped pose;
    // otherwise the head drawn later would completely hide them.
    const float handSpread = 1.10f + 0.10f * ease;
    const float armSway = std::sin (animationPhase * 1.65f) * faceR * 0.025f * ease;
    const float leftHandX = faceCX - faceR * handSpread - armSway;
    const float rightHandX = faceCX + faceR * handSpread + armSway;
    const float armControlY1 = faceCY + faceR * (0.76f - 0.49f * ease);
    const float armControlY2 = handY + faceR * (-0.015f + 0.69f * ease);

    juce::Path leftArm;
    leftArm.startNewSubPath (leftShoulderX, shoulderY);
    leftArm.cubicTo (faceCX - faceR * (0.88f + 0.11f * ease), armControlY1,
                     leftHandX + faceR * 0.13f, armControlY2, leftHandX, handY);
    juce::Path rightArm;
    rightArm.startNewSubPath (rightShoulderX, shoulderY);
    rightArm.cubicTo (faceCX + faceR * (0.88f + 0.11f * ease), armControlY1,
                      rightHandX - faceR * 0.13f, armControlY2, rightHandX, handY);
    const float armThickness = 10.0f + 1.8f * ease;
    strokeRoundedPath (g, leftArm, juce::Colours::black.withAlpha (0.25f), armThickness + 4.0f);
    strokeRoundedPath (g, rightArm, juce::Colours::black.withAlpha (0.25f), armThickness + 4.0f);
    strokeRoundedPath (g, leftArm, juce::Colour (0xff812746), armThickness + 0.8f);
    strokeRoundedPath (g, rightArm, juce::Colour (0xff812746), armThickness + 0.8f);
    strokeRoundedPath (g, leftArm, juce::Colour (0xfff04491), armThickness - 1.8f);
    strokeRoundedPath (g, rightArm, juce::Colour (0xfff04491), armThickness - 1.8f);
    strokeRoundedPath (g, leftArm, juce::Colour (0xffffb0d0).withAlpha (0.68f), 1.25f);
    strokeRoundedPath (g, rightArm, juce::Colour (0xffffb0d0).withAlpha (0.68f), 1.25f);

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

    // Layer a few swept orange tufts into the top rim, giving the silhouette
    // more of the animated orange-haired mascot look instead of a flat ring.
    for (int tuft = 0; tuft < 7; ++tuft)
    {
        const float a = juce::MathConstants<float>::pi * (1.18f + 0.107f * (float) tuft);
        const float nextA = a + 0.11f;
        const float tipA = a + 0.055f;
        juce::Path hair;
        hair.startNewSubPath (faceCX + std::cos (a) * faceR * 0.91f,
                              faceCY + std::sin (a) * faceR * 0.91f);
        hair.cubicTo (faceCX + std::cos (a) * faceR * 1.01f,
                      faceCY + std::sin (a) * faceR * 1.01f,
                      faceCX + std::cos (tipA) * faceR * 1.09f,
                      faceCY + std::sin (tipA) * faceR * 1.09f,
                      faceCX + std::cos (tipA) * faceR * 1.09f,
                      faceCY + std::sin (tipA) * faceR * 1.09f);
        hair.cubicTo (faceCX + std::cos (nextA) * faceR * 1.03f,
                      faceCY + std::sin (nextA) * faceR * 1.03f,
                      faceCX + std::cos (nextA) * faceR * 0.95f,
                      faceCY + std::sin (nextA) * faceR * 0.95f,
                      faceCX + std::cos (nextA) * faceR * 0.91f,
                      faceCY + std::sin (nextA) * faceR * 0.91f);
        hair.closeSubPath();
        g.setColour (tuft % 2 == 0 ? juce::Colour (0xffffad36) : juce::Colour (0xffff9628));
        g.fillPath (hair);
        g.setColour (juce::Colour (0xffffd078).withAlpha (0.60f));
        g.strokePath (hair, juce::PathStrokeType (0.9f));
    }

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
    const float eyeW = faceR * (0.125f + 0.085f * ease);
    const float eyeH = faceR * (0.155f + 0.300f * ease);
    const float eyeY = faceCY - faceR * (0.325f + 0.030f * ease);
    const float eyeOffset = faceR * 0.43f;
    for (int side = -1; side <= 1; side += 2)
    {
        const float ex = faceCX + (float) side * eyeOffset - eyeW * 0.5f;
        juce::Path brow;
        brow.startNewSubPath (ex - eyeW * 0.02f, eyeY - faceR * 0.08f);
        brow.quadraticTo (ex + eyeW * 0.50f, eyeY - faceR * (0.075f + 0.075f * ease),
                          ex + eyeW * 1.02f, eyeY - faceR * (0.025f + 0.035f * ease));
        strokeRoundedPath (g, brow, juce::Colour (0xffb84a6f).withAlpha (0.48f), 2.0f);
        const float eyeCX = ex + eyeW * 0.5f;
        const float eyeCY = eyeY + eyeH * 0.5f;
        const float eyeTilt = (float) side * (0.025f + 0.17f * ease);
        juce::Path eyeSocket;
        eyeSocket.addEllipse (ex - eyeW * 0.10f, eyeY - eyeH * 0.025f,
                              eyeW * 1.20f, eyeH * 1.05f);
        eyeSocket.applyTransform (juce::AffineTransform::rotation (eyeTilt, eyeCX, eyeCY));
        g.setColour (juce::Colour (0xfff6e9dd).withAlpha (0.92f));
        g.fillPath (eyeSocket);
        juce::Path eye;
        eye.addEllipse (ex, eyeY, eyeW, eyeH);
        eye.applyTransform (juce::AffineTransform::rotation (eyeTilt, eyeCX, eyeCY));
        g.setColour (juce::Colour (0xff7c2447).withAlpha (0.58f));
        g.fillPath (eye, juce::AffineTransform::translation (0.0f, 1.5f));
        g.setColour (juce::Colour (0xffa42f59));
        g.fillPath (eye);

        // Tiny pupils look low and inward when the mascot is miserable, then
        // lift as it approaches the ecstatic pose.
        const float pupilW = eyeW * (0.17f + 0.035f * ease);
        const float pupilH = eyeH * (0.19f + 0.035f * ease);
        const float pupilX = eyeCX - pupilW * 0.5f - (float) side * eyeW * (0.055f + 0.025f * ease);
        const float pupilY = eyeCY - pupilH * 0.5f + eyeH * (0.115f - 0.27f * ease);
        juce::Path pupil;
        pupil.addEllipse (pupilX, pupilY, pupilW, pupilH);
        pupil.applyTransform (juce::AffineTransform::rotation (eyeTilt, eyeCX, eyeCY));
        g.setColour (juce::Colour (0xff45182d).withAlpha (0.86f));
        g.fillPath (pupil);
        g.setColour (juce::Colour (0xffffd1df).withAlpha (0.88f));
        g.fillEllipse (ex + eyeW * 0.18f, eyeY + eyeH * 0.11f,
                       eyeW * 0.24f, eyeH * 0.18f);
        g.setColour (juce::Colours::white.withAlpha (0.36f));
        g.fillEllipse (ex + eyeW * 0.62f, eyeY + eyeH * 0.58f,
                       eyeW * 0.09f, eyeH * 0.08f);

        const float lidAmount = juce::jmax (0.64f * (1.0f - ease),
                                            0.98f * blinkAmount * (1.0f - 0.85f * ease));
        if (lidAmount > 0.025f)
        {
            juce::Path lid;
            lid.startNewSubPath (ex - 1.5f, eyeY - 1.5f);
            lid.lineTo (ex + eyeW + 1.5f, eyeY - 1.5f);
            lid.lineTo (ex + eyeW + 1.5f, eyeY + eyeH * lidAmount);
            lid.quadraticTo (ex + eyeW * 0.50f, eyeY + eyeH * (lidAmount + 0.10f),
                             ex - 1.5f, eyeY + eyeH * lidAmount);
            lid.closeSubPath();
            g.setColour (juce::Colour (0xffd8f0df));
            g.fillPath (lid);
            juce::Path lidCrease;
            lidCrease.startNewSubPath (ex + 1.0f, eyeY + eyeH * lidAmount);
            lidCrease.quadraticTo (ex + eyeW * 0.50f, eyeY + eyeH * (lidAmount + 0.10f),
                                   ex + eyeW - 1.0f, eyeY + eyeH * lidAmount);
            strokeRoundedPath (g, lidCrease, juce::Colour (0xffa42f59).withAlpha (0.38f), 1.0f);
        }
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
    const float noseTop = faceCY - faceR * (0.32f + 0.035f * ease);
    const float noseBottom = faceCY + faceR * (0.12f + 0.035f * ease);
    const float noseHalfWidth = faceR * (0.12f + 0.020f * ease);
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

    const float mouthHalfWidth = faceR * (0.43f + 0.35f * ease);
    const float mouthTopY = faceCY + faceR * (0.205f + 0.012f * t);
    const float openT = juce::jlimit (0.0f, 1.0f, (t - 0.18f) / 0.82f);
    const float mouthDepth = faceR * (0.04f + 0.44f * openT);

    if (t < 0.30f)
    {
        // At zero the corners fall and the centre rises into a frown. As intensity
        // enters, the frown gently turns over into a small smile before opening.
        const float frownAmount = 1.0f - juce::jlimit (0.0f, 1.0f, t / 0.30f);
        const float cornerY = mouthTopY + faceR * 0.065f * frownAmount;
        const float centreY = mouthTopY - faceR * 0.042f * frownAmount
                            + faceR * 0.075f * (1.0f - frownAmount);
        juce::Path closedExpressionShadow;
        closedExpressionShadow.startNewSubPath (faceCX - mouthHalfWidth, cornerY + 2.0f);
        closedExpressionShadow.cubicTo (faceCX - mouthHalfWidth * 0.40f, centreY + 1.5f,
                                        faceCX + mouthHalfWidth * 0.40f, centreY + 1.5f,
                                        faceCX + mouthHalfWidth, cornerY + 2.0f);
        strokeRoundedPath (g, closedExpressionShadow, juce::Colour (0xff812746), faceR * 0.075f);
        juce::Path closedExpression;
        closedExpression.startNewSubPath (faceCX - mouthHalfWidth, cornerY);
        closedExpression.cubicTo (faceCX - mouthHalfWidth * 0.40f, centreY,
                                  faceCX + mouthHalfWidth * 0.40f, centreY,
                                  faceCX + mouthHalfWidth, cornerY);
        strokeRoundedPath (g, closedExpression, pink, faceR * 0.052f);
        strokeRoundedPath (g, closedExpression, juce::Colour (0xffffb7d2).withAlpha (0.75f), 1.15f);
    }
    else
    {
        // Crescent mouth opens smoothly from a tiny smile into an exaggerated grin.
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

        // Continuous enamel strip follows the upper lip. Fine seams suggest
        // separate rounded teeth without making them look like floating tiles.
        const float upperToothH = juce::jmax (4.0f, faceR * (0.040f + 0.066f * openT));
        const auto upperToothTop = [&] (float nx)
        {
            return mouthTopY + topCurveDepth * (0.72f * (1.0f - nx * nx) + 0.08f) + 1.2f;
        };
        const auto upperToothHeight = [&] (float nx)
        {
            return upperToothH * (0.88f + 0.12f * (1.0f - std::abs (nx)));
        };
        juce::Path teethBand;
        constexpr int toothCurveSteps = 24;
        for (int step = 0; step <= toothCurveSteps; ++step)
        {
            const float nx = juce::jmap ((float) step / (float) toothCurveSteps, -0.80f, 0.80f);
            const float px = faceCX + nx * mouthHalfWidth;
            const float py = upperToothTop (nx);
            if (step == 0) teethBand.startNewSubPath (px, py);
            else teethBand.lineTo (px, py);
        }
        for (int step = toothCurveSteps; step >= 0; --step)
        {
            const float nx = juce::jmap ((float) step / (float) toothCurveSteps, -0.80f, 0.80f);
            teethBand.lineTo (faceCX + nx * mouthHalfWidth,
                              upperToothTop (nx) + upperToothHeight (nx));
        }
        teethBand.closeSubPath();
        g.setColour (juce::Colour (0xff4b172b).withAlpha (0.45f));
        g.fillPath (teethBand, juce::AffineTransform::translation (0.5f, 1.3f));
        g.setColour (juce::Colour (0xffffe9df));
        g.fillPath (teethBand);
        g.setColour (juce::Colours::white.withAlpha (0.52f));
        juce::Path toothHighlight;
        toothHighlight.startNewSubPath (faceCX - mouthHalfWidth * 0.78f, upperToothTop (-0.78f) + 1.0f);
        for (int step = 1; step <= toothCurveSteps; ++step)
        {
            const float nx = juce::jmap ((float) step / (float) toothCurveSteps, -0.78f, 0.78f);
            toothHighlight.lineTo (faceCX + nx * mouthHalfWidth, upperToothTop (nx) + 1.0f);
        }
        strokeRoundedPath (g, toothHighlight, juce::Colour (0xffffffff).withAlpha (0.58f), 1.0f);
        g.setColour (juce::Colour (0xffa45c69).withAlpha (0.64f));
        for (int seam = 1; seam < 8; ++seam)
        {
            const float nx = juce::jmap ((float) seam / 8.0f, -0.80f, 0.80f);
            const float sx = faceCX + nx * mouthHalfWidth;
            const float sy = upperToothTop (nx) + upperToothHeight (nx) * 0.80f;
            g.drawLine (sx, upperToothTop (nx) + 1.5f, sx, sy, 1.0f);
        }

        // Six small lower teeth frame the tongue without becoming detached tiles.
        const float lowerToothW = mouthHalfWidth * 0.105f;
        const float lowerToothH = juce::jmax (3.0f, faceR * 0.034f);
        for (int tooth = 0; tooth < 6; ++tooth)
        {
            const float side = tooth < 3 ? -1.0f : 1.0f;
            const int localTooth = tooth % 3;
            const float position = localTooth == 0 ? 0.76f : (localTooth == 1 ? 0.59f : 0.42f);
            const float tx = faceCX + side * mouthHalfWidth * position;
            const float ty = mouthTopY + mouthDepth * (0.86f - 0.025f * position);
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

    // Restore the panel coordinate system before drawing the fixed footer.
    g.restoreState();

    // Branded footer inspired by the original product-title strip while keeping
    // this clean-room build identified as the Inspired edition.
    g.setColour (juce::Colour (0xffe7e2dd).withAlpha (0.82f));
    g.setFont (juce::Font (juce::FontOptions (27.0f, juce::Font::bold)));
    g.drawText ("ENDLESS SMILE INSPIRED", 20, getHeight() - 57,
                juce::roundToInt (w * 0.56f), 34, juce::Justification::centredLeft,
                true);
    g.setColour (juce::Colour (0xff171719));
    g.setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::bold)));
    g.drawText ("MIX", mix.getX() - 34, mix.getY(), 30, mix.getHeight(),
                juce::Justification::centredRight);

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
    const int presetY = juce::roundToInt ((float) bounds.getHeight() * 0.66f);
    preset.setBounds (leftMargin + 44, presetY + 1, controlWidth - 88, 68);
    previousPresetButton.setBounds (leftMargin - 12, presetY + 14, 56, 42);
    nextPresetButton.setBounds (leftMargin + controlWidth - 44, presetY + 14, 56, 42);
    mix.setBounds (leftMargin + 28, presetY + 75, controlWidth - 54, 30);
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
