#include "PluginEditor.h"
#include "BinaryData.h"
#include <cmath>

namespace
{
    // Presets 1-7 are the original seven; 8-16 are the nine graveyard presets.
    constexpr int numPresets = 16;


    void strokeRoundedPath (juce::Graphics& g, const juce::Path& path,
                            juce::Colour colour, float thickness)
    {
        g.setColour (colour);
        g.strokePath (path, juce::PathStrokeType (thickness,
                        juce::PathStrokeType::curved,
                        juce::PathStrokeType::rounded));
    }
}

EndlessSmileInspiredLookAndFeel::EndlessSmileInspiredLookAndFeel()
{
    // Knob-face artwork embedded at build time (Assets/KnobEmblem.png via CMake).
    emblemImage = juce::ImageFileFormat::loadFrom (BinaryData::KnobEmblem_png,
                                                   (size_t) BinaryData::KnobEmblem_pngSize);
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
    const float t = juce::jlimit (0.0f, 1.0f, dialDisplayPosition >= 0.0f ? dialDisplayPosition
                                                                          : sliderPosProportional);
    const float current = start + (end - start) * t;
    const float knobR = radius * 0.62f;
    const float emblemR = radius - 14.0f;   // the emblem fills the plate, just inside the ticks
    const float readoutY = cy + radius + 8.0f;

    // ---- Static layer: identical for every slider value ---------------------
    const auto drawStaticLayer = [&] (juce::Graphics& gg)
    {
        // Soft drop shadow separates the control from the flat grey panel.
        gg.setColour (juce::Colours::black.withAlpha (0.16f));
        gg.fillEllipse (cx - radius - 2.0f, cy - radius + 3.0f,
                        (radius + 2.0f) * 2.0f, (radius + 2.0f) * 2.0f);

        // Machined plate behind the arc.
        juce::ColourGradient plate (juce::Colour (0xff626061), cx - radius * 0.35f, cy - radius * 0.45f,
                                    juce::Colour (0xff454344), cx + radius * 0.75f, cy + radius * 0.85f, true);
        gg.setGradientFill (plate);
        gg.fillEllipse (cx - radius + 3.0f, cy - radius + 3.0f,
                        (radius - 3.0f) * 2.0f, (radius - 3.0f) * 2.0f);
        gg.setColour (juce::Colours::white.withAlpha (0.07f));
        gg.drawEllipse (cx - radius + 3.5f, cy - radius + 3.5f,
                        (radius - 3.5f) * 2.0f, (radius - 3.5f) * 2.0f, 1.0f);

        // Recessed track with tiny calibration ticks.
        juce::Path track;
        track.addCentredArc (cx, cy, radius, radius, 0.0f, start, end, true);
        strokeRoundedPath (gg, track, juce::Colour (0xff1e2a2e), 7.2f);

        for (int tick = 0; tick <= 20; ++tick)
        {
            const float a = start + (end - start) * (float) tick / 20.0f;
            const float inner = radius - ((tick % 5 == 0) ? 12.0f : 7.0f);
            const float outer = radius - 2.0f;
            const float x1 = cx + std::cos (a - quarterTurn) * inner;
            const float y1 = cy + std::sin (a - quarterTurn) * inner;
            const float x2 = cx + std::cos (a - quarterTurn) * outer;
            const float y2 = cy + std::sin (a - quarterTurn) * outer;
            gg.setColour (juce::Colour (0xffc2c3c3).withAlpha (tick % 5 == 0 ? 0.56f : 0.23f));
            gg.drawLine (x1, y1, x2, y2, tick % 5 == 0 ? 1.4f : 0.8f);
        }

        if (emblemImage.isValid())
        {
            // The emblem is the knob face: a circular image with a feathered edge.
            gg.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
            gg.drawImage (emblemImage,
                          juce::Rectangle<float> (cx - emblemR, cy - emblemR, emblemR * 2.0f, emblemR * 2.0f),
                          juce::RectanglePlacement::stretchToFit);
        }
        else
        {
        // Brushed-metal bezel and radial-gradient knob body.
        gg.setColour (juce::Colour (0xffd1d0d0).withAlpha (0.94f));
        gg.fillEllipse (cx - knobR - 3.2f, cy - knobR - 3.2f,
                        (knobR + 3.2f) * 2.0f, (knobR + 3.2f) * 2.0f);
        gg.setColour (juce::Colour (0xff777879));
        gg.drawEllipse (cx - knobR - 1.5f, cy - knobR - 1.5f,
                        (knobR + 1.5f) * 2.0f, (knobR + 1.5f) * 2.0f, 1.0f);

        juce::ColourGradient body (juce::Colour (0xffe4e5e7), cx - knobR * 0.48f, cy - knobR * 0.70f,
                                   juce::Colour (0xff6f7175), cx + knobR * 0.78f, cy + knobR * 0.78f, true);
        body.addColour (0.38, juce::Colour (0xffc6c7ca));
        body.addColour (0.72, juce::Colour (0xff97999d));
        gg.setGradientFill (body);
        gg.fillEllipse (cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f);

        // Fine concentric rings give the disc a spun, brushed-metal finish.
        for (float r = 2.5f; r < knobR - 1.0f; r += 2.2f)
        {
            const bool light = ((int) (r / 2.2f)) % 2 == 0;
            gg.setColour ((light ? juce::Colours::white : juce::Colours::black).withAlpha (light ? 0.07f : 0.05f));
            gg.drawEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f, 0.7f);
        }

        // Two broad anisotropic reflections, as on spun brushed metal.
        for (int k = 0; k < 2; ++k)
        {
            const float a0 = k == 0 ? 0.30f : 3.45f;
            juce::Path wide, narrow;
            wide.addPieSegment (cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f, a0, a0 + 0.62f, 0.0f);
            narrow.addPieSegment (cx - knobR, cy - knobR, knobR * 2.0f, knobR * 2.0f, a0 + 0.16f, a0 + 0.42f, 0.0f);
            gg.setColour (juce::Colours::white.withAlpha (0.08f));
            gg.fillPath (wide);
            gg.setColour (juce::Colours::white.withAlpha (0.10f));
            gg.fillPath (narrow);
        }

        // Subtle glints on the upper edge of the metal ring.
        juce::Path bezelGlint;
        bezelGlint.addCentredArc (cx, cy, knobR + 1.0f, knobR + 1.0f, 0.0f,
                                  juce::MathConstants<float>::pi * 1.08f,
                                  juce::MathConstants<float>::pi * 1.88f, true);
        strokeRoundedPath (gg, bezelGlint, juce::Colours::white.withAlpha (0.28f), 1.4f);
        }

        // End-of-travel labels.
        gg.setColour (juce::Colour (0xffc2c3c3).withAlpha (0.72f));
        gg.setFont (juce::Font (juce::FontOptions (9.0f, juce::Font::bold)));
        for (int endIndex = 0; endIndex < 2; ++endIndex)
        {
            const float a = endIndex == 0 ? start : end;
            const float lx = cx + std::cos (a - quarterTurn) * radius;
            const float ly = cy + std::sin (a - quarterTurn) * radius;
            gg.drawText (endIndex == 0 ? "MIN" : "MAX",
                         juce::Rectangle<int> (juce::roundToInt (lx) - 20, juce::roundToInt (ly) + 11, 40, 11),
                         juce::Justification::centred);
        }

        // Readout frame and the INTENSITY label (the number itself is live).
        gg.setColour (juce::Colour (0xff282629).withAlpha (0.16f));
        gg.fillRoundedRectangle (cx - 46.0f, readoutY + 1.0f, 92.0f, 24.0f, 1.5f);
        gg.setColour (juce::Colour (0xffd4d0d0).withAlpha (0.82f));
        gg.drawRoundedRectangle (cx - 45.0f, readoutY, 90.0f, 24.0f, 1.0f, 0.9f);

        gg.setColour (juce::Colour (0xffe9e2cf).withAlpha (0.94f));
        gg.setFont (juce::Font (juce::FontOptions (14.5f, juce::Font::bold)));
        gg.drawText ("INTENSITY",
                     juce::Rectangle<int> (juce::roundToInt (cx - 72.0f),
                                           juce::roundToInt (readoutY + 25.0f), 144, 20),
                     juce::Justification::centred);
    };

    const float scale = juce::jmax (1.0f, (float) g.getInternalContext().getPhysicalPixelScaleFactor());
    const juce::Rectangle<int> area (x, y, width, height);
    const int cacheW = juce::jmax (1, slider.getWidth());
    const int cacheH = juce::jmax (1, slider.getHeight());
    const bool cacheStale = ! dialCache.isValid()
                         || dialCacheArea != area
                         || dialCacheWidth != cacheW || dialCacheHeight != cacheH
                         || dialCacheScale != scale
                         || dialCacheStart != start || dialCacheEnd != end;
    if (cacheStale)
    {
        dialCache = juce::Image (juce::Image::ARGB,
                                 juce::roundToInt ((float) cacheW * scale) + 1,
                                 juce::roundToInt ((float) cacheH * scale) + 1, true);
        juce::Graphics cacheGraphics (dialCache);
        cacheGraphics.addTransform (juce::AffineTransform::scale (scale));
        drawStaticLayer (cacheGraphics);
        dialCacheArea = area;
        dialCacheWidth = cacheW;
        dialCacheHeight = cacheH;
        dialCacheScale = scale;
        dialCacheStart = start;
        dialCacheEnd = end;
    }
    g.drawImageTransformed (dialCache, juce::AffineTransform::scale (1.0f / scale));

    // ---- Dynamic layer: value arc, marker, paddle and the live number --------
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
    g.setColour (juce::Colours::white.withAlpha (0.45f));
    g.fillEllipse (markerX - 2.6f, markerY - 4.2f, 3.6f, 2.4f);

    if (! emblemImage.isValid())
    {
    // Large glossy black blade that overhangs the disc: a lit upper facet and a darker
    // lower facet meet at a centre ridge, narrowing to a sharp tip on the +x end
    // (which points at the current value).
    const float pointerAngle = current;
    const float bladeL = 1.20f;   // overhang along the blade
    const float bladeT = 1.10f;   // thickness
    const auto bx = [&] (float f) { return cx + knobR * f * bladeL; };
    const auto by = [&] (float f) { return cy + knobR * f * bladeT; };
    juce::Path grip;
    grip.startNewSubPath (bx (-0.95f), by (-0.26f));
    grip.quadraticTo (bx (-0.20f), by (-0.72f), bx (0.55f), by (-0.20f));
    grip.lineTo (bx (0.98f), by (0.02f));
    grip.lineTo (bx (0.50f), by (0.28f));
    grip.quadraticTo (bx (-0.25f), by (0.64f), bx (-0.95f), by (0.22f));
    grip.closeSubPath();
    juce::Path gripUpper;
    gripUpper.startNewSubPath (bx (-0.95f), by (-0.26f));
    gripUpper.quadraticTo (bx (-0.20f), by (-0.72f), bx (0.55f), by (-0.20f));
    gripUpper.lineTo (bx (0.98f), by (0.02f));
    gripUpper.lineTo (bx (-0.95f), by (-0.02f));
    gripUpper.closeSubPath();
    const auto rotateBlade = juce::AffineTransform::rotation (pointerAngle - quarterTurn, cx, cy);
    grip.applyTransform (rotateBlade);
    gripUpper.applyTransform (rotateBlade);

    g.setColour (juce::Colours::black.withAlpha (0.60f));
    g.fillPath (grip, juce::AffineTransform::translation (2.2f, 3.4f));
    juce::ColourGradient lowerFacet (juce::Colour (0xff1b1c1f), cx - knobR * 0.3f, cy - knobR * 0.2f,
                                     juce::Colour (0xff030304), cx + knobR * 0.35f, cy + knobR * 0.7f, false);
    g.setGradientFill (lowerFacet);
    g.fillPath (grip);
    juce::ColourGradient upperFacet (juce::Colour (0xff5a5d63), cx - knobR * 0.5f, cy - knobR * 0.7f,
                                     juce::Colour (0xff131517), cx + knobR * 0.4f, cy + knobR * 0.1f, false);
    g.setGradientFill (upperFacet);
    g.fillPath (gripUpper);
    g.setColour (juce::Colour (0xff85878c).withAlpha (0.90f));
    g.strokePath (grip, juce::PathStrokeType (0.9f));

    // Bright ridge line down the middle of the blade.
    juce::Path ridge;
    ridge.startNewSubPath (bx (-0.90f), by (-0.015f));
    ridge.lineTo (bx (0.90f), by (0.012f));
    ridge.applyTransform (rotateBlade);
    strokeRoundedPath (g, ridge, juce::Colours::white.withAlpha (0.30f), 1.0f);

    // Light tip facet so the pointing end is unmistakable.
    juce::Path gripTip;
    gripTip.startNewSubPath (bx (0.98f), by (0.02f));
    gripTip.lineTo (bx (0.72f), by (-0.10f));
    gripTip.lineTo (bx (0.74f), by (0.14f));
    gripTip.closeSubPath();
    gripTip.applyTransform (rotateBlade);
    g.setColour (juce::Colour (0xffdadbdd).withAlpha (0.92f));
    g.fillPath (gripTip);

    // Curved specular ridge along the blade.
    juce::Path gripGlint;
    gripGlint.startNewSubPath (bx (-0.72f), by (-0.12f));
    gripGlint.quadraticTo (bx (-0.10f), by (-0.36f), bx (0.58f), by (-0.09f));
    gripGlint.applyTransform (rotateBlade);
    strokeRoundedPath (g, gripGlint, juce::Colours::white.withAlpha (0.50f), 2.2f);
    g.setColour (juce::Colour (0xffa5a8ab).withAlpha (0.92f));
    g.fillEllipse (cx - 2.8f, cy - 2.8f, 5.6f, 5.6f);
    }
    else
    {
        // Power-up: the emblem is dim and dormant at zero and reaches full neon at maximum.
        const float dim = (1.0f - t) * 0.62f;
        if (dim > 0.002f)
        {
            g.setColour (juce::Colours::black.withAlpha (dim));
            g.fillEllipse (cx - emblemR, cy - emblemR, emblemR * 2.0f, emblemR * 2.0f);
        }

        const juce::Colour neon (0xff4dff2a);
        if (t > 0.01f)
        {
            // Green glow bleeding outwards from the rim as intensity rises.
            const float outer = emblemR + 16.0f;
            const double rim = (double) (emblemR / outer);
            juce::ColourGradient glow (neon.withAlpha (0.0f), cx, cy,
                                       neon.withAlpha (0.0f), cx + outer, cy, true);
            glow.addColour (rim - 0.002, neon.withAlpha (0.0f));
            glow.addColour (rim, neon.withAlpha (0.30f * t));
            g.setGradientFill (glow);
            g.fillEllipse (cx - outer, cy - outer, outer * 2.0f, outer * 2.0f);
        }

        // Pointer: a neon spike on the rim that turns with the value.
        const float dx = std::cos (current - quarterTurn);
        const float dy = std::sin (current - quarterTurn);
        const float px = -dy;
        const float py = dx;
        juce::Path pointer;
        pointer.startNewSubPath (cx + dx * (emblemR - 2.0f), cy + dy * (emblemR - 2.0f));
        pointer.lineTo (cx + dx * (emblemR + 11.0f) + px * 5.5f, cy + dy * (emblemR + 11.0f) + py * 5.5f);
        pointer.lineTo (cx + dx * (emblemR + 11.0f) - px * 5.5f, cy + dy * (emblemR + 11.0f) - py * 5.5f);
        pointer.closeSubPath();
        g.setColour (neon.withAlpha (0.30f));
        g.strokePath (pointer, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved));
        g.setColour (neon);
        g.fillPath (pointer);
        g.setColour (juce::Colour (0xff031a07));
        g.strokePath (pointer, juce::PathStrokeType (1.0f));
        juce::Path core;
        core.startNewSubPath (cx + dx * (emblemR + 1.0f), cy + dy * (emblemR + 1.0f));
        core.lineTo (cx + dx * (emblemR + 8.0f), cy + dy * (emblemR + 8.0f));
        strokeRoundedPath (g, core, juce::Colours::white.withAlpha (0.75f), 1.2f);
    }

    g.setColour (juce::Colour (0xfffbfafa));
    g.setFont (juce::Font (juce::FontOptions (13.5f)));
    g.drawText (juce::String (slider.getValue(), 2),
                juce::Rectangle<int> (juce::roundToInt (cx - 43.0f),
                                      juce::roundToInt (readoutY), 86, 24),
                juce::Justification::centred);
}

void EndlessSmileInspiredLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height,
    bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box)
{
    juce::ignoreUnused (isButtonDown, buttonX, buttonY, buttonW, buttonH);
    juce::ignoreUnused (box);
    // The category follows the sliding tab itself, so the label changes as the tab
    // passes each preset's notch rather than the instant a preset is chosen.
    const int tabIndex = juce::jlimit (0, numPresets - 1,
                                       juce::roundToInt (cassetteTabPosition * (float) (numPresets - 1)));
    const juce::String category = tabIndex <= 1 ? "SOFT"
        : (tabIndex <= 3 ? "MEDIUM" : (tabIndex <= 5 ? "HARD" : (tabIndex == 6 ? "EXTREME" : "GRAVEYARD")));
    const juce::Colour tabColour = tabIndex <= 1 ? juce::Colour (0xffffa13b)
        : (tabIndex <= 3 ? juce::Colour (0xff46b8d5)
        : (tabIndex <= 5 ? juce::Colour (0xffff5a4d)
        : (tabIndex == 6 ? juce::Colour (0xffb06cff) : juce::Colour (0xffbff7d0))));

    // Cast-metal pink housing and lower shadow.
    g.setColour (juce::Colours::black.withAlpha (0.34f));
    g.fillRoundedRectangle (3.0f, 4.0f, (float) width - 6.0f, (float) height - 5.0f, 5.0f);
    juce::ColourGradient casing (juce::Colour (0xff7ee89a), 0.0f, 0.0f,
                                  juce::Colour (0xff1c7a42), (float) width, (float) height, false);
    casing.addColour (0.42, juce::Colour (0xff35b862));
    g.setGradientFill (casing);
    g.fillRoundedRectangle (1.0f, 0.5f, (float) width - 4.0f, (float) height - 7.0f, 5.5f);
    g.setColour (juce::Colour (0xffc3f7d3).withAlpha (0.82f));
    g.drawRoundedRectangle (1.5f, 1.0f, (float) width - 5.0f, (float) height - 8.0f, 5.0f, 1.0f);
    g.setColour (juce::Colour (0xff0f4a28).withAlpha (0.85f));
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
    // One fixed width for every category, so the tab never changes size as it slides.
    const float tabWidth = 80.0f;
    // The tab rides along a timeline with one notch per preset, so it slides
    // left to right as you step from the first preset to the last.
    const float tabMargin = 56.0f;
    const float travel = juce::jmax (1.0f, (float) width - 2.0f * tabMargin);
    const float tabCentre = tabMargin + cassetteTabPosition * travel;
    const float tabX = juce::jlimit (14.0f, (float) width - tabWidth - 14.0f, tabCentre - tabWidth * 0.5f);
    g.setColour (juce::Colour (0xff134a2a).withAlpha (0.35f));
    g.fillRect (tabMargin, 10.5f, travel, 1.0f);
    for (int notch = 0; notch < numPresets; ++notch)
    {
        const float nx = tabMargin + travel * (float) notch / (float) (numPresets - 1);
        g.setColour (juce::Colour (0xff134a2a).withAlpha (0.75f));
        g.fillEllipse (nx - 1.6f, 9.4f, 3.2f, 3.2f);
    }

    g.setColour (juce::Colour (0xff0b3a1d).withAlpha (0.55f));
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
        g.setColour (juce::Colour (0xff0f4a28));
        g.fillEllipse (sx - 1.7f, (float) height * 0.53f - 1.7f, 3.4f, 3.4f);
        g.setColour (juce::Colour (0xffd9ffe6));
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
    const juce::Colour arrowTop = down ? juce::Colour (0xffff9a3a)
                                       : (highlighted ? juce::Colour (0xffffcb78) : juce::Colour (0xffffb04d));
    const juce::Colour arrowBottom = down ? juce::Colour (0xffd9560a)
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
    lookAndFeel.setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff0e1913));
    lookAndFeel.setColour (juce::PopupMenu::textColourId, juce::Colour (0xffe9e2cf));
    lookAndFeel.setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xff2fae5a));
    lookAndFeel.setColour (juce::PopupMenu::highlightedTextColourId, juce::Colour (0xff08140c));
    lookAndFeel.setColour (juce::PopupMenu::headerTextColourId, juce::Colour (0xffbff7d0));

    intensity.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    intensity.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    intensity.setRange (0.0, 110.0, 0.1);
    intensity.setNumDecimalPlacesToDisplay (2);
    intensity.setMouseDragSensitivity (170);               // shorter drag for the full range
    intensity.setDoubleClickReturnValue (true, 0.0);        // double-click resets to zero
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
        "DADA DYNAMITE",
        "MIDNIGHT HOWL",
        "BANSHEE SCREAM",
        "GHOST WHISPER",
        "RATTLING BONES",
        "CRYPT DRONE",
        "WAILING WRAITH",
        "ZOMBIE GROAN",
        "COFFIN CREAK",
        "HAUNTED BELLS"
    };
    // Group the dropdown by category. Headings are not items, so ids still run 1..16
    // and the parameter attachment is unaffected.
    for (int i = 0; i < presetNames.size(); ++i)
    {
        if (i == 0) preset.addSectionHeading ("SOFT");
        if (i == 2) preset.addSectionHeading ("MEDIUM");
        if (i == 4) preset.addSectionHeading ("HARD");
        if (i == 6) preset.addSectionHeading ("EXTREME");
        if (i == 7) preset.addSectionHeading ("GRAVEYARD");
        preset.addItem (presetNames[i], i + 1);
    }
    preset.setJustificationType (juce::Justification::centredLeft);
    preset.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff35b862));
    preset.setColour (juce::ComboBox::textColourId, juce::Colour (0xff30252b));
    preset.setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff17622f));
    preset.setColour (juce::ComboBox::arrowColourId, juce::Colour (0xff30252b));
    preset.setTooltip ("Choose a build-up profile. Previous/next arrows also cycle the presets.");
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
            case 8: preset.setTooltip ("Midnight Howl - Graveyard: wind howling through a drifting resonant band."); break;
            case 9: preset.setTooltip ("Banshee Scream - Graveyard: a rising, shrieking voice with echo."); break;
            case 10: preset.setTooltip ("Ghost Whisper - Graveyard: breathy whispers and a haunting echo."); break;
            case 11: preset.setTooltip ("Rattling Bones - Graveyard: dry bone clicks that get denser as intensity rises."); break;
            case 12: preset.setTooltip ("Crypt Drone - Graveyard: a low, growing drone with a moving vowel."); break;
            case 13: preset.setTooltip ("Wailing Wraith - Graveyard: a wavering, mournful wail with a trailing echo."); break;
            case 14: preset.setTooltip ("Zombie Groan - Graveyard: a low, growling groan that swells with intensity."); break;
            case 15: preset.setTooltip ("Coffin Creak - Graveyard: a long, resonant creak that wanders in pitch."); break;
            case 16: preset.setTooltip ("Haunted Bells - Graveyard: slow, ringing bell strikes that come faster as intensity rises."); break;
            default: preset.setTooltip ("Choose one of the build-up profiles."); break;
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
        preset.setSelectedId (id <= 1 ? numPresets : id - 1, juce::sendNotificationSync);
    };
    addAndMakeVisible (previousPresetButton);

    nextPresetButton.setName ("preset-next");
    nextPresetButton.setButtonText ("");
    nextPresetButton.setTooltip ("Next preset");
    nextPresetButton.setMouseCursor (juce::MouseCursor::PointingHandCursor);
    nextPresetButton.onClick = [this]
    {
        const int id = preset.getSelectedId();
        preset.setSelectedId (id >= numPresets ? 1 : id + 1, juce::sendNotificationSync);
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
    tabSlide = (float) (juce::jmax (1, preset.getSelectedId()) - 1) / (float) (numPresets - 1);
    lookAndFeel.setCassetteTabPosition (tabSlide);
    knobDisplay = (float) intensity.valueToProportionOfLength (intensity.getValue());
    lookAndFeel.setDialDisplayPosition (knobDisplay);
    buildFogTexture();

    // The mascot follows the knob instantly: no easing, repaint right away.
    intensity.onValueChange = [this]
    {
        // State is set at once; the 60 Hz timer paints it. Painting here on every
        // mouse-move event could mean well over 100 big repaints a second while dragging.
        animatedIntensity = juce::jlimit (0.0f, 1.0f, (float) intensity.getValue() / 110.0f);
        characterDirty = true;
    };
    startTimerHz (60);
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

    // The static panel comes from a cache stored at physical-pixel resolution, so every
    // repaint is a plain 1:1 copy instead of a scaled one.
    paintStartMs = juce::Time::getMillisecondCounterHiRes();
    pixelScale = juce::jmax (1.0f, (float) g.getInternalContext().getPhysicalPixelScaleFactor());
    if (! backgroundCache.isValid() || backgroundCacheScale != pixelScale)
    {
        rebuildBackgroundCache (pixelScale);
        skipCostSample = true;
    }
    ensureFogImages();
    if (backgroundCache.isValid())
        g.drawImageTransformed (backgroundCache, juce::AffineTransform::scale (1.0f / pixelScale));

    g.setColour (juce::Colour (0xff07080a));
    g.drawRoundedRectangle (bounds.reduced (8.0f), 11.0f, 3.0f);
    g.setColour (juce::Colour (0xff7d8794).withAlpha (0.60f));
    g.drawRoundedRectangle (bounds.reduced (13.0f), 8.0f, 1.2f);
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.drawLine (20.0f, 21.0f, w - 20.0f, 21.0f, 1.0f);

    // The character is drawn behind the real JUCE intensity slider. This keeps
    // arms behind the dial and gives the whole mascot a consistent silhouette.
    const float faceR = juce::jmin (w * 0.125f, h * 0.208f);
    const float bounceX = std::sin (animationPhase * 0.83f) * faceR * 0.010f * ease;
    const float bounceY = std::cos (animationPhase * 1.17f) * faceR * 0.012f * ease;
    const float faceCX = w * 0.758f + bounceX;
    const float faceCY = h * juce::jmap (ease, 0.535f, 0.515f) + bounceY;
    const float torsoTop = faceCY + faceR * (0.70f - 0.025f * ease);
    const float torsoBottom = h * (0.835f + 0.020f * ease);
    const float legBottom = h * (0.915f + 0.012f * ease);   // ankle
    const float shoeBottom = legBottom + h * (0.024f + 0.002f * ease);

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
    g.fillEllipse (faceCX - faceR * 0.70f, shoeBottom - 3.0f, faceR * 1.40f, 9.0f);

    // Legs: femur, shin and a proper foot with toe divisions.
    for (int side = -1; side <= 1; side += 2)
    {
        const float sd = (float) side;
        const float legX = faceCX + sd * faceR * 0.17f;
        const float hipY = torsoBottom - 4.0f;
        const float kneeY = hipY + (legBottom - hipY) * 0.48f;
        drawBone (legX, hipY, legX, kneeY, 6.6f);
        drawBone (legX, kneeY, legX, legBottom, 5.4f);

        const float footLen = faceR * 0.34f;
        const float footX = side < 0 ? legX - footLen + 5.0f : legX - 5.0f;
        const float footTop = legBottom - 1.0f;
        const float footH = juce::jmax (7.0f, shoeBottom - legBottom);
        g.setColour (boneEdge);
        g.fillRoundedRectangle (footX - 0.8f, footTop - 0.8f, footLen + 1.6f, footH + 1.6f, 4.5f);
        g.setColour (boneLight);
        g.fillRoundedRectangle (footX, footTop, footLen, footH, 3.8f);
        g.setColour (boneEdge.withAlpha (0.55f));
        for (int toe = 1; toe <= 3; ++toe)
        {
            const float f = 0.12f * (float) toe + 0.04f;
            const float tx = side < 0 ? footX + footLen * f : footX + footLen * (1.0f - f);
            g.drawLine (tx, footTop + footH * 0.30f, tx, footTop + footH - 1.0f, 1.0f);
        }
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
    const float visibleTorsoTop = faceCY + faceR * 0.93f;
    const float cageTop = visibleTorsoTop - 2.0f;
    g.setColour (socketDark.withAlpha (0.62f));
    g.fillEllipse (faceCX - faceR * 0.44f, cageTop, faceR * 0.88f, (pelvisY + 2.0f) - cageTop);

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
            strokeRoundedPath (g, ribPath, boneEdge, 5.2f);
            strokeRoundedPath (g, ribPath, boneLight, 3.6f);
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

    // Soft sheen and faint jaw lines (drawn before the mouth so it covers them).
    juce::ColourGradient sheen (juce::Colours::white.withAlpha (0.38f),
                                faceCX - faceR * 0.40f, faceCY - faceR * 0.62f,
                                juce::Colours::white.withAlpha (0.0f),
                                faceCX + faceR * 0.02f, faceCY - faceR * 0.62f, true);
    g.saveState();
    juce::Path skullClip;
    skullClip.addEllipse (faceCX - faceR, faceCY - faceR, faceR * 2.0f, faceR * 2.0f);
    g.reduceClipRegion (skullClip, juce::AffineTransform());
    g.setGradientFill (sheen);
    g.fillEllipse (faceCX - faceR * 0.80f, faceCY - faceR * 0.95f, faceR * 0.80f, faceR * 0.60f);
    g.restoreState();
    for (int side = -1; side <= 1; side += 2)
    {
        const float sd = (float) side;
        juce::Path jaw;
        jaw.startNewSubPath (faceCX + sd * faceR * 0.80f, faceCY + faceR * 0.10f);
        jaw.quadraticTo (faceCX + sd * faceR * 0.78f, faceCY + faceR * 0.55f,
                         faceCX + sd * faceR * 0.30f, faceCY + faceR * 0.84f);
        strokeRoundedPath (g, jaw, boneEdge.withAlpha (0.35f), 1.5f);
    }

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
    const float lidAmount = juce::jmax (0.64f * (1.0f - ease) * (1.0f - ease),
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
        const float browY = eyeY - faceR * (0.06f + 0.05f * ease);
        juce::Path brow;
        brow.startNewSubPath (eyeCX - sd * eyeW * 0.62f, browY - faceR * 0.07f * (1.0f - ease));
        brow.quadraticTo (eyeCX, browY - faceR * (0.05f + 0.025f * ease),
                          eyeCX + sd * eyeW * 0.62f, browY + faceR * 0.01f * (1.0f - ease));
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

    // Footer: title, short accent rule and subtitle, all kept clear of the inner frame line.
    g.setColour (juce::Colour (0xffe9e2cf).withAlpha (0.92f));
    g.setFont (juce::Font (juce::FontOptions (24.0f, juce::Font::bold)));
    g.drawText ("SKELETON GRAVEYARD", 26, getHeight() - 62,
                juce::roundToInt (w * 0.46f), 28, juce::Justification::centredLeft, true);
    g.setColour (juce::Colour (0xff39afd2).withAlpha (0.85f));
    g.fillRect (27.0f, (float) getHeight() - 33.0f, 44.0f, 2.0f);
    g.setColour (juce::Colour (0xffddd8d0).withAlpha (0.78f));
    g.setFont (juce::Font (juce::FontOptions (9.5f, juce::Font::bold)));
    g.drawText ("SKELLY AUDIO  |  BUILD-UP / TRANSITION EFFECT",
                26, getHeight() - 30, getWidth() - 52, 12, juce::Justification::centredLeft);
}

void EndlessSmileInspiredAudioProcessorEditor::resized()
{
    // Caches are rebuilt lazily in paint(), once the real pixel scale is known.
    backgroundCache = juce::Image();
    fogBottomImage = juce::Image();
    fogTopImage = juce::Image();

    const auto bounds = getLocalBounds();
    const int dialWidth = juce::jmin (280, juce::roundToInt ((float) bounds.getHeight() * 0.44f));
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

void EndlessSmileInspiredAudioProcessorEditor::buildFogTexture()
{
    // A small, horizontally tileable fog texture. Every u-dependent term uses a whole
    // number of cycles, so the left and right edges meet seamlessly when it scrolls.
    constexpr int fw = 512;
    constexpr int fh = 96;
    const float twoPi = juce::MathConstants<float>::twoPi;
    fogTexture = juce::Image (juce::Image::ARGB, fw, fh, true);
    juce::Image::BitmapData data (fogTexture, juce::Image::BitmapData::writeOnly);
    for (int y = 0; y < fh; ++y)
    {
        const float v = (float) y / (float) fh;
        const float edge = std::sin (juce::MathConstants<float>::pi * v);
        for (int x = 0; x < fw; ++x)
        {
            const float u = (float) x / (float) fw;
            float n = 0.50f * std::sin (twoPi * (2.0f * u + 0.10f * std::sin (twoPi * (v + 0.3f))) + 1.3f);
            n += 0.32f * std::sin (twoPi * (3.0f * u + 0.15f) + 4.0f * v + 0.7f);
            n += 0.22f * std::sin (twoPi * (5.0f * u) - 5.0f * v + 2.1f);
            n += 0.15f * std::sin (twoPi * (8.0f * u + 0.37f) + 7.0f * v);
            float d = juce::jlimit (0.0f, 1.0f, 0.5f + 0.5f * n);
            d = d * d * (3.0f - 2.0f * d);
            data.setPixelColour (x, y, juce::Colour::fromFloatRGBA (0.66f, 0.74f, 0.80f,
                                                                    juce::jlimit (0.0f, 1.0f, d * edge)));
        }
    }
}

juce::Rectangle<int> EndlessSmileInspiredAudioProcessorEditor::getFogBottomBand() const
{
    return { 0, juce::roundToInt ((float) getHeight() * 0.60f),
             getWidth(), juce::roundToInt ((float) getHeight() * 0.38f) };
}

juce::Rectangle<int> EndlessSmileInspiredAudioProcessorEditor::getFogTopBand() const
{
    return { 0, juce::roundToInt ((float) getHeight() * 0.04f),
             getWidth(), juce::roundToInt ((float) getHeight() * 0.20f) };
}

void EndlessSmileInspiredAudioProcessorEditor::ensureFogImages()
{
    if (! fogTexture.isValid() || getWidth() <= 0 || getHeight() <= 0)
        return;

    const int widthPx = juce::roundToInt ((float) getWidth() * pixelScale);
    if (fogBottomImage.isValid() && fogTopImage.isValid()
        && fogImageScale == pixelScale && fogImageWidth == widthPx)
        return;

    // Stretch the small tileable texture to the full window width once, so scrolling it
    // later never needs any scaling or filtering.
    const auto makeBand = [&] (juce::Rectangle<int> band)
    {
        const int heightPx = juce::jmax (1, juce::roundToInt ((float) band.getHeight() * pixelScale));
        juce::Image image (juce::Image::ARGB, widthPx, heightPx, true);
        juce::Graphics ig (image);
        ig.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
        ig.drawImage (fogTexture, 0, 0, widthPx, heightPx,
                      0, 0, fogTexture.getWidth(), fogTexture.getHeight());
        return image;
    };
    fogBottomImage = makeBand (getFogBottomBand());
    fogTopImage = makeBand (getFogTopBand());
    fogImageScale = pixelScale;
    fogImageWidth = widthPx;
    skipCostSample = true;
}

void EndlessSmileInspiredAudioProcessorEditor::paintOverChildren (juce::Graphics& g)
{
    if (fogBottomImage.isValid() && fogTopImage.isValid())
    {
        // Two drifting layers at different speeds and strengths, drawn over everything so
        // the mist passes in front of the controls and the skeleton as well as behind them.
        const auto drawLayer = [&] (const juce::Image& image, juce::Rectangle<int> band,
                                    float offset, float opacity)
        {
            const int widthPx = image.getWidth();
            const int shiftPx = juce::roundToInt (offset * (float) widthPx) % widthPx;
            g.setOpacity (opacity);
            for (int copy = 0; copy < 2; ++copy)
            {
                const float xLogical = (float) (copy * widthPx - shiftPx) / pixelScale;
                g.drawImageTransformed (image, juce::AffineTransform::scale (1.0f / pixelScale)
                                                   .translated (xLogical, (float) band.getY()));
            }
        };
        drawLayer (fogBottomImage, getFogBottomBand(), fogOffsetA, 0.55f);
        drawLayer (fogTopImage, getFogTopBand(), fogOffsetB, 0.30f);
    }

    // Measure how long this whole repaint pass took (background, controls, mascot and fog).
    const float cost = (float) (juce::Time::getMillisecondCounterHiRes() - paintStartMs);
    if (skipCostSample)
        skipCostSample = false;        // the pass that built the caches is not representative
    else
        paintPeakMs = juce::jmax (cost, paintPeakMs);
}

void EndlessSmileInspiredAudioProcessorEditor::rebuildBackgroundCache (float scale)
{
    if (getWidth() <= 0 || getHeight() <= 0)
        return;

    const float w = (float) getWidth();
    const float h = (float) getHeight();
    backgroundCache = juce::Image (juce::Image::ARGB, juce::roundToInt (w * scale),
                                   juce::roundToInt (h * scale), true);
    backgroundCacheScale = scale;
    juce::Graphics g (backgroundCache);
    g.addTransform (juce::AffineTransform::scale (scale));

    // ---- Graveyard at night (drawn once per size; never repainted per frame) ----
    juce::ColourGradient sky (juce::Colour (0xff05070b), 0.0f, 0.0f,
                              juce::Colour (0xff1d2733), 0.0f, h * 0.90f, false);
    sky.addColour (0.55, juce::Colour (0xff0d121a));
    g.setGradientFill (sky);
    g.fillAll();

    // Sickly green mist glowing up from the horizon.
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1fbf6a).withAlpha (0.0f), 0.0f, h * 0.50f,
                                             juce::Colour (0xff1fbf6a).withAlpha (0.22f), 0.0f, h * 0.90f, false));
    g.fillRect (0.0f, h * 0.50f, w, h * 0.50f);

    juce::Random random (91);
    for (int i = 0; i < 120; ++i)
    {
        const float px = random.nextFloat() * w;
        const float py = random.nextFloat() * h * 0.62f;
        const float size = 0.8f + random.nextFloat() * 1.3f;
        g.setColour (juce::Colours::white.withAlpha (0.10f + random.nextFloat() * 0.45f));
        g.fillEllipse (px, py, size, size);
    }

    // Moon with a soft halo, between the dial and the skeleton.
    const float moonX = w * 0.585f;
    const float moonY = h * 0.17f;
    const float moonR = h * 0.072f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffdfe8f2).withAlpha (0.26f), moonX, moonY,
                                             juce::Colour (0xffdfe8f2).withAlpha (0.0f), moonX + moonR * 4.4f, moonY, true));
    g.fillEllipse (moonX - moonR * 4.4f, moonY - moonR * 4.4f, moonR * 8.8f, moonR * 8.8f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff0f4f9), moonX - moonR * 0.4f, moonY - moonR * 0.4f,
                                             juce::Colour (0xffbcc7d6), moonX + moonR, moonY + moonR, false));
    g.fillEllipse (moonX - moonR, moonY - moonR, moonR * 2.0f, moonR * 2.0f);
    g.setColour (juce::Colour (0xff8996a8).withAlpha (0.20f));
    g.fillEllipse (moonX - moonR * 0.55f, moonY - moonR * 0.25f, moonR * 0.45f, moonR * 0.45f);
    g.fillEllipse (moonX + moonR * 0.15f, moonY + moonR * 0.30f, moonR * 0.32f, moonR * 0.32f);
    g.fillEllipse (moonX + moonR * 0.20f, moonY - moonR * 0.58f, moonR * 0.24f, moonR * 0.24f);

    // A few bats across the moon.
    const auto drawBat = [&g] (float bx, float by, float size)
    {
        juce::Path bat;
        bat.startNewSubPath (bx - size, by - size * 0.15f);
        bat.quadraticTo (bx - size * 0.55f, by - size * 0.75f, bx, by);
        bat.quadraticTo (bx + size * 0.55f, by - size * 0.75f, bx + size, by - size * 0.15f);
        bat.quadraticTo (bx + size * 0.5f, by + size * 0.10f, bx, by + size * 0.35f);
        bat.quadraticTo (bx - size * 0.5f, by + size * 0.10f, bx - size, by - size * 0.15f);
        bat.closeSubPath();
        g.setColour (juce::Colour (0xff04060a));
        g.fillPath (bat);
    };
    drawBat (moonX + moonR * 1.5f, moonY - moonR * 0.4f, h * 0.020f);
    drawBat (moonX - moonR * 0.9f, moonY - moonR * 1.5f, h * 0.014f);
    drawBat (moonX + moonR * 2.5f, moonY + moonR * 0.9f, h * 0.012f);

    const float baseGround = h * 0.91f;

    // Faint ghosts drifting in the dark.
    const auto drawGhost = [&g] (float gx, float gy, float gs, float alpha)
    {
        juce::Path ghost;
        ghost.startNewSubPath (gx - gs * 0.5f, gy + gs * 0.9f);
        ghost.lineTo (gx - gs * 0.5f, gy);
        ghost.quadraticTo (gx - gs * 0.5f, gy - gs * 0.62f, gx, gy - gs * 0.62f);
        ghost.quadraticTo (gx + gs * 0.5f, gy - gs * 0.62f, gx + gs * 0.5f, gy);
        ghost.lineTo (gx + gs * 0.5f, gy + gs * 0.9f);
        ghost.quadraticTo (gx + gs * 0.33f, gy + gs * 0.72f, gx + gs * 0.17f, gy + gs * 0.9f);
        ghost.quadraticTo (gx, gy + gs * 0.72f, gx - gs * 0.17f, gy + gs * 0.9f);
        ghost.quadraticTo (gx - gs * 0.33f, gy + gs * 0.72f, gx - gs * 0.5f, gy + gs * 0.9f);
        ghost.closeSubPath();
        g.setColour (juce::Colour (0xffcfe9ff).withAlpha (alpha));
        g.fillPath (ghost);
        g.setColour (juce::Colour (0xff05070b).withAlpha (juce::jmin (1.0f, alpha * 4.0f)));
        g.fillEllipse (gx - gs * 0.22f, gy - gs * 0.16f, gs * 0.12f, gs * 0.17f);
        g.fillEllipse (gx + gs * 0.10f, gy - gs * 0.16f, gs * 0.12f, gs * 0.17f);
        g.fillEllipse (gx - gs * 0.07f, gy + gs * 0.10f, gs * 0.14f, gs * 0.20f);
    };
    drawGhost (w * 0.225f, h * 0.30f, h * 0.075f, 0.13f);
    drawGhost (w * 0.135f, h * 0.52f, h * 0.055f, 0.10f);

    // Haunted house on the rise between the cassette and the skeleton.
    {
        const float hx = w * 0.470f;
        const float hw = w * 0.075f;
        const float hh = h * 0.105f;
        const float hy = baseGround + 4.0f - hh;
        const float towerW = w * 0.022f;
        const float towerH = h * 0.175f;
        const float towerX = hx + hw - towerW * 0.4f;
        const float towerY = baseGround + 4.0f - towerH;
        const juce::Colour silhouette (0xff04060a);
        g.setColour (silhouette);
        g.fillRect (hx, hy, hw, hh);
        juce::Path roof;
        roof.startNewSubPath (hx - hw * 0.06f, hy);
        roof.lineTo (hx + hw * 0.5f, hy - hh * 0.55f);
        roof.lineTo (hx + hw * 1.06f, hy);
        roof.closeSubPath();
        g.fillPath (roof);
        g.fillRect (towerX, towerY, towerW, towerH);
        juce::Path spire;
        spire.startNewSubPath (towerX - towerW * 0.18f, towerY);
        spire.lineTo (towerX + towerW * 0.5f, towerY - towerH * 0.30f);
        spire.lineTo (towerX + towerW * 1.18f, towerY);
        spire.closeSubPath();
        g.fillPath (spire);

        const auto glowWindow = [&g] (float wx, float wy, float ww, float wh)
        {
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xffd8ff7a).withAlpha (0.30f), wx + ww * 0.5f, wy + wh * 0.5f,
                                                     juce::Colour (0xffd8ff7a).withAlpha (0.0f), wx + ww * 0.5f + ww * 2.6f, wy + wh * 0.5f, true));
            g.fillEllipse (wx - ww * 2.1f, wy - wh * 1.6f, ww * 5.2f, wh * 4.2f);
            g.setColour (juce::Colour (0xffe9ff9a).withAlpha (0.92f));
            g.fillRect (wx, wy, ww, wh);
        };
        glowWindow (hx + hw * 0.18f, hy + hh * 0.25f, hw * 0.16f, hh * 0.26f);
        glowWindow (hx + hw * 0.62f, hy + hh * 0.25f, hw * 0.16f, hh * 0.26f);
        glowWindow (towerX + towerW * 0.30f, towerY + towerH * 0.16f, towerW * 0.40f, towerH * 0.12f);
    }

    // Twisted dead trees framing the scene.
    const auto branch = [&g, &random] (auto&& self, float bx, float by, float angle,
                                       float len, float thick, int depth) -> void
    {
        const float ex = bx + std::sin (angle) * len;
        const float ey = by - std::cos (angle) * len;
        g.setColour (juce::Colour (0xff05070a));
        g.drawLine (bx, by, ex, ey, thick);
        if (depth <= 0 || len < 6.0f)
            return;
        const int children = depth > 2 ? 2 : (random.nextBool() ? 2 : 1);
        for (int c = 0; c < children; ++c)
        {
            const float spread = (c == 0 ? -1.0f : 1.0f) * (0.35f + random.nextFloat() * 0.55f);
            self (self, ex, ey, angle + spread, len * (0.62f + random.nextFloat() * 0.18f),
                  thick * 0.66f, depth - 1);
        }
    };
    branch (branch, w * 0.020f, baseGround + 8.0f, 0.10f, h * 0.20f, 7.0f, 6);
    branch (branch, w * 0.985f, baseGround + 8.0f, -0.12f, h * 0.17f, 6.0f, 6);

    // Tombstones and crosses, then the ground so their bases sink into it.
    struct Stone { float x, width, height; int kind; };
    const Stone stones[] = {
        { 0.060f, 0.040f, 0.085f, 0 }, { 0.170f, 0.030f, 0.070f, 1 }, { 0.300f, 0.042f, 0.090f, 0 },
        { 0.420f, 0.034f, 0.075f, 2 }, { 0.590f, 0.034f, 0.074f, 1 },
        { 0.905f, 0.040f, 0.110f, 0 }, { 0.948f, 0.030f, 0.078f, 2 }, { 0.975f, 0.026f, 0.066f, 1 }
    };
    const float groundY = h * 0.91f;
    for (const auto& stone : stones)
    {
        const float sx = w * stone.x;
        const float sw = h * stone.width;
        const float sh = h * stone.height;
        const float top = groundY - sh;
        juce::Path shape;
        if (stone.kind == 0)        // arched headstone
        {
            shape.addRectangle (sx, top + sw * 0.5f, sw, sh - sw * 0.5f + 6.0f);
            shape.addEllipse (sx, top, sw, sw);
        }
        else if (stone.kind == 1)   // cross
        {
            shape.addRectangle (sx + sw * 0.38f, top, sw * 0.24f, sh + 6.0f);
            shape.addRectangle (sx, top + sh * 0.22f, sw, sw * 0.22f);
        }
        else                        // plain slab with a chipped corner
        {
            shape.startNewSubPath (sx, top + sw * 0.35f);
            shape.lineTo (sx + sw * 0.4f, top);
            shape.lineTo (sx + sw, top);
            shape.lineTo (sx + sw, groundY + 6.0f);
            shape.lineTo (sx, groundY + 6.0f);
            shape.closeSubPath();
        }
        g.setColour (juce::Colour (0xff0f1318));
        g.fillPath (shape);
        g.setColour (juce::Colour (0xff5d6b7a).withAlpha (0.16f));
        g.strokePath (shape, juce::PathStrokeType (1.0f));
    }

    juce::Path ground;
    ground.startNewSubPath (0.0f, h * 0.925f);
    ground.cubicTo (w * 0.28f, h * 0.885f, w * 0.60f, h * 0.945f, w, h * 0.90f);
    ground.lineTo (w, h);
    ground.lineTo (0.0f, h);
    ground.closeSubPath();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0a0d10), 0.0f, h * 0.88f,
                                             juce::Colour (0xff040506), 0.0f, h, false));
    g.fillPath (ground);

    // Drifting ground fog.
    for (int band = 0; band < 3; ++band)
    {
        const float fy = h * (0.70f + 0.095f * (float) band);
        const float fh = h * 0.15f;
        juce::ColourGradient fog (juce::Colour (0xff9fb0c0).withAlpha (0.0f), 0.0f, fy - fh * 0.5f,
                                  juce::Colour (0xff9fb0c0).withAlpha (0.0f), 0.0f, fy + fh * 0.5f, false);
        fog.addColour (0.5, juce::Colour (0xff9fb0c0).withAlpha (0.075f));
        g.setGradientFill (fog);
        g.fillRect (0.0f, fy - fh * 0.5f, w, fh);
    }

    // Cobweb in the top-left corner.
    {
        const float webCx = 16.0f, webCy = 16.0f, webR = h * 0.17f;
        const float quarter = juce::MathConstants<float>::halfPi;
        g.setColour (juce::Colour (0xffd9e4f0).withAlpha (0.24f));
        for (int spoke = 0; spoke <= 6; ++spoke)
        {
            const float a = (float) spoke * quarter / 6.0f;
            g.drawLine (webCx, webCy, webCx + std::cos (a) * webR, webCy + std::sin (a) * webR, 0.8f);
        }
        for (int ring = 1; ring <= 4; ++ring)
        {
            const float r = webR * (float) ring / 4.5f;
            juce::Path strand;
            for (int spoke = 0; spoke <= 6; ++spoke)
            {
                const float a = (float) spoke * quarter / 6.0f;
                const float px = webCx + std::cos (a) * r;
                const float py = webCy + std::sin (a) * r;
                if (spoke == 0)
                    strand.startNewSubPath (px, py);
                else
                {
                    const float am = a - quarter / 12.0f;
                    strand.quadraticTo (webCx + std::cos (am) * r * 0.90f, webCy + std::sin (am) * r * 0.90f, px, py);
                }
            }
            g.strokePath (strand, juce::PathStrokeType (0.8f));
        }
    }

    // Something watching from the dark.
    {
        const float ex = w * 0.085f, ey = h * 0.30f, gap = h * 0.028f;
        for (int eye = -1; eye <= 1; eye += 2)
        {
            const float px = ex + (float) eye * gap;
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xffcfff3a).withAlpha (0.35f), px, ey,
                                                     juce::Colour (0xffcfff3a).withAlpha (0.0f), px + h * 0.030f, ey, true));
            g.fillEllipse (px - h * 0.030f, ey - h * 0.030f, h * 0.060f, h * 0.060f);
            g.setColour (juce::Colour (0xffe6ff6a));
            g.fillEllipse (px - h * 0.008f, ey - h * 0.005f, h * 0.016f, h * 0.011f);
        }
    }

    // Vignette, then a little deterministic grain.
    g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack, w * 0.5f, h * 0.45f,
                                             juce::Colours::black.withAlpha (0.55f), 0.0f, 0.0f, true));
    g.fillAll();
    for (int i = 0; i < 360; ++i)
    {
        const float px = random.nextFloat() * w;
        const float py = random.nextFloat() * h;
        const float alpha = random.nextFloat() * 0.020f;
        g.setColour ((i % 3 == 0 ? juce::Colours::white : juce::Colours::black).withAlpha (alpha));
        g.fillEllipse (px, py, 1.0f, 1.0f);
    }
}

juce::Rectangle<int> EndlessSmileInspiredAudioProcessorEditor::getCharacterBounds() const
{
    // Everything the animated character can touch lies right of this line, clear of
    // the dial and cassette, so animation never forces those controls to repaint.
    const int x = juce::roundToInt ((float) getWidth() * 0.52f);
    return { x, 0, getWidth() - x, getHeight() };
}

void EndlessSmileInspiredAudioProcessorEditor::timerCallback()
{
    // 60 Hz. The controls always come first: while the knob is being dragged or the
    // preset tab is sliding, decorative repaints (fog, idle sway) are paused, so the
    // main thread is free to keep the controls smooth.
    const float tabTarget = (float) (juce::jmax (1, preset.getSelectedId()) - 1) / (float) (numPresets - 1);
    const bool tabSliding = tabSlide != tabTarget;
    if (tabSliding)
    {
        tabSlide += (tabTarget - tabSlide) * 0.18f;
        if (std::abs (tabTarget - tabSlide) < 0.0015f)
            tabSlide = tabTarget;
        lookAndFeel.setCassetteTabPosition (tabSlide);
        preset.repaint();
    }

    // The dial glides toward the real value (cosmetic only: the number and the
    // parameter are always exact), which hides the steps between mouse movements.
    const float knobTarget = (float) intensity.valueToProportionOfLength (intensity.getValue());
    const bool knobMoving = knobDisplay != knobTarget;
    if (knobMoving)
    {
        knobDisplay += (knobTarget - knobDisplay) * 0.42f;
        if (std::abs (knobTarget - knobDisplay) < 0.0008f)
            knobDisplay = knobTarget;
        lookAndFeel.setDialDisplayPosition (knobDisplay);
        intensity.repaint();
    }
    const bool busy = tabSliding || knobMoving || intensity.isMouseButtonDown();

    // Slow fog drift (time-based, so it stays smooth whatever the repaint rate).
    fogOffsetA += 0.020f / 60.0f;
    fogOffsetB += 0.034f / 60.0f;
    if (fogOffsetA >= 1.0f) fogOffsetA -= 1.0f;
    if (fogOffsetB >= 1.0f) fogOffsetB -= 1.0f;
    if (! busy && fogInterval > 0 && (animationFrame % fogInterval) == 0)
    {
        repaint (getFogBottomBand());
        repaint (getFogTopBand());
    }

    // Self-tuning once a second: the slower the slowest recent repaint pass, the less
    // often the fog is repainted. The peak decays with time, so it recovers by itself.
    paintPeakMs *= 0.985f;
    if ((animationFrame % 60) == 0)
        fogInterval = paintPeakMs > 18.0f ? 12 : (paintPeakMs > 11.0f ? 6 : 3);

    const float previousIntensity = animatedIntensity;
    const float previousBlink = blinkAmount;
    animatedIntensity = juce::jlimit (0.0f, 1.0f, (float) intensity.getValue() / 110.0f);
    if (animatedIntensity != previousIntensity)
        characterDirty = true;

    // Idle motion and blinking keep their original pace (tuned at 30 Hz, so half-steps here).
    animationPhase += (0.035f + 0.055f * animatedIntensity) * 0.5f;
    if (animationPhase >= juce::MathConstants<float>::twoPi)
        animationPhase -= juce::MathConstants<float>::twoPi;

    ++animationFrame;
    if (framesUntilBlink <= 0)
    {
        blinkAmount = 1.0f;
        framesUntilBlink = 184 + (animationFrame % 122);
    }
    else
    {
        blinkAmount *= 0.742f;
        if (blinkAmount < 0.005f)
            blinkAmount = 0.0f;
        --framesUntilBlink;
    }

    // Mascot: follow the knob promptly (every tick when idle, every other tick while the
    // knob is held), and sway at 30 Hz only when nothing is being dragged.
    const bool animating = animatedIntensity > 0.002f || blinkAmount > 0.0f || previousBlink > 0.0f;
    const bool swayTick = animating && ! busy && (animationFrame & 1) == 0;
    const bool followTick = characterDirty && (! busy || (animationFrame & 1) == 0);
    if (followTick || swayTick)
    {
        characterDirty = false;
        repaint (getCharacterBounds());
    }
}
