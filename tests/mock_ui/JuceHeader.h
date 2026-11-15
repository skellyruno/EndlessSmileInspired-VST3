// Type-check stand-in for the JUCE GUI classes used by PluginEditor.cpp. Signatures follow
// JUCE 8; bodies are empty. It only proves the editor is type-correct against those signatures.
#pragma once
#define MOCK_UI 1
#include "../mock/JuceHeader.h"
#include <functional>
#include <cstring>

namespace juce
{
    using uint8 = std::uint8_t;
    using int64 = std::int64_t;
    enum NotificationType { dontSendNotification, sendNotification, sendNotificationSync, sendNotificationAsync };

    template <class T> struct Point
    {
        T x {}, y {};
        Point() = default; Point (T a, T b) : x (a), y (b) {}
        T getX() const { return x; } T getY() const { return y; }
        Point operator+ (Point o) const { return { x + o.x, y + o.y }; }
        Point operator- (Point o) const { return { x - o.x, y - o.y }; }
        Point operator* (T k) const { return { x * k, y * k }; } Point operator/ (T k) const { return { x / k, y / k }; }
        template <class U> Point<U> toFloat() const { return { (U) x, (U) y }; }
        Point<float> toFloat() const { return { (float) x, (float) y }; }
        Point<int> toInt() const { return { (int) x, (int) y }; }
        T getDistanceFrom (Point o) const { return std::sqrt ((x - o.x) * (x - o.x) + (y - o.y) * (y - o.y)); }
    };
    template <class T> struct Rectangle
    {
        T x {}, y {}, w {}, h {};
        Rectangle() = default; Rectangle (T X, T Y, T W, T H) : x (X), y (Y), w (W), h (H) {}
        template <class U> explicit Rectangle (Rectangle<U> o) : x ((T) o.x), y ((T) o.y), w ((T) o.w), h ((T) o.h) {}
        T getX() const { return x; } T getY() const { return y; } T getWidth() const { return w; } T getHeight() const { return h; }
        T getRight() const { return x + w; } T getBottom() const { return y + h; }
        T getCentreX() const { return x + w / 2; } T getCentreY() const { return y + h / 2; }
        Point<T> getCentre() const { return { getCentreX(), getCentreY() }; }
        bool isEmpty() const { return w <= 0 || h <= 0; }
        Rectangle reduced (T d) const { return { x + d, y + d, w - 2 * d, h - 2 * d }; }
        Rectangle reduced (T dx, T dy) const { return { x + dx, y + dy, w - 2 * dx, h - 2 * dy }; }
        Rectangle expanded (T d) const { return { x - d, y - d, w + 2 * d, h + 2 * d }; }
        Rectangle expanded (T dx, T dy) const { return { x - dx, y - dy, w + 2 * dx, h + 2 * dy }; }
        Rectangle translated (T dx, T dy) const { return { x + dx, y + dy, w, h }; }
        Rectangle withSize (T W, T H) const { return { x, y, W, H }; }
        Rectangle withHeight (T H) const { return { x, y, w, H }; }
        Rectangle withWidth (T W) const { return { x, y, W, h }; }
        Rectangle withY (T Y) const { return { x, Y, w, h }; }
        Rectangle withX (T X) const { return { X, y, w, h }; }
        Rectangle withTrimmedTop (T d) const { return { x, y + d, w, h - d }; }
        Rectangle withTrimmedBottom (T d) const { return { x, y, w, h - d }; }
        Rectangle withTrimmedLeft (T d) const { return { x + d, y, w - d, h }; }
        Rectangle withTrimmedRight (T d) const { return { x, y, w - d, h }; }
        Rectangle withSizeKeepingCentre (T W, T H) const { return { getCentreX() - W / 2, getCentreY() - H / 2, W, H }; }
        Rectangle removeFromTop (T d) { Rectangle r { x, y, w, d }; y += d; h -= d; return r; }
        Rectangle removeFromBottom (T d) { Rectangle r { x, y + h - d, w, d }; h -= d; return r; }
        Rectangle removeFromLeft (T d) { Rectangle r { x, y, d, h }; x += d; w -= d; return r; }
        Rectangle removeFromRight (T d) { Rectangle r { x + w - d, y, d, h }; w -= d; return r; }
        Rectangle getUnion (Rectangle o) const { const T X = std::min (x, o.x), Y = std::min (y, o.y);
                                                 return { X, Y, std::max (getRight(), o.getRight()) - X, std::max (getBottom(), o.getBottom()) - Y }; }
        Rectangle getIntersection (Rectangle o) const { const T X = std::max (x, o.x), Y = std::max (y, o.y);
                                                        return { X, Y, std::max ((T) 0, std::min (getRight(), o.getRight()) - X), std::max ((T) 0, std::min (getBottom(), o.getBottom()) - Y) }; }
        bool intersects (Rectangle o) const { return ! getIntersection (o).isEmpty(); }
        bool contains (Point<T> p) const { return p.x >= x && p.y >= y && p.x < x + w && p.y < y + h; }
        bool contains (T px, T py) const { return contains (Point<T> (px, py)); }
        Rectangle<float> toFloat() const { return { (float) x, (float) y, (float) w, (float) h }; }
        Rectangle<int> toNearestInt() const { return { (int) std::lround (x), (int) std::lround (y), (int) std::lround (w), (int) std::lround (h) }; }
        Rectangle<int> getSmallestIntegerContainer() const { return { (int) std::floor (x), (int) std::floor (y), (int) std::ceil (getRight()) - (int) std::floor (x), (int) std::ceil (getBottom()) - (int) std::floor (y) }; }
        bool operator== (const Rectangle& o) const { return x == o.x && y == o.y && w == o.w && h == o.h; }
        bool operator!= (const Rectangle& o) const { return ! (*this == o); }
    };

    struct Justification { int flags; Justification (int f) : flags (f) {}
        enum { left = 1, right = 2, horizontallyCentred = 4, top = 8, bottom = 16, verticallyCentred = 32,
               centred = 36, centredLeft = 33, centredRight = 34, centredTop = 12, centredBottom = 20, topLeft = 9, topRight = 10 }; };

    struct AffineTransform
    {
        float m00 = 1, m01 = 0, m02 = 0, m10 = 0, m11 = 1, m12 = 0;
        AffineTransform() = default;
        AffineTransform (float a, float b, float c, float d, float e, float f) : m00 (a), m01 (b), m02 (c), m10 (d), m11 (e), m12 (f) {}
        static AffineTransform translation (float dx, float dy) { return { 1, 0, dx, 0, 1, dy }; }
        static AffineTransform scale (float s) { return { s, 0, 0, 0, s, 0 }; }
        static AffineTransform scale (float sx, float sy) { return { sx, 0, 0, 0, sy, 0 }; }
        static AffineTransform scale (float sx, float sy, float cx, float cy) { return { sx, 0, cx - sx * cx, 0, sy, cy - sy * cy }; }
        static AffineTransform rotation (float r) { return { std::cos (r), -std::sin (r), 0, std::sin (r), std::cos (r), 0 }; }
        static AffineTransform rotation (float r, float cx, float cy) { return rotation (r).translated (0, 0).followedBy (translation (cx, cy)).followedBy (translation (-cx, -cy)); }
        AffineTransform followedBy (const AffineTransform&) const { return *this; }
        AffineTransform translated (float dx, float dy) const { return followedBy (translation (dx, dy)); }
        AffineTransform scaled (float s) const { return followedBy (scale (s)); }
        AffineTransform scaled (float sx, float sy) const { return followedBy (scale (sx, sy)); }
        AffineTransform rotated (float r) const { return followedBy (rotation (r)); }
        AffineTransform rotated (float r, float cx, float cy) const { return followedBy (rotation (r, cx, cy)); }
        static AffineTransform identity() { return {}; }
    };

    class Colour
    {
    public:
        Colour() = default;
        Colour (uint32 argb) : v (argb) {}
        Colour (uint8 r, uint8 g, uint8 b) : v (0xff000000u | ((uint32) r << 16) | ((uint32) g << 8) | b) {}
        Colour (uint8 r, uint8 g, uint8 b, uint8 a) : v (((uint32) a << 24) | ((uint32) r << 16) | ((uint32) g << 8) | b) {}
        Colour (uint8 r, uint8 g, uint8 b, float a) : Colour (r, g, b, (uint8) (a * 255.0f)) {}
        static Colour fromFloatRGBA (float r, float g, float b, float a) { return Colour ((uint8) (r * 255), (uint8) (g * 255), (uint8) (b * 255), a); }
        static Colour fromRGB (uint8 r, uint8 g, uint8 b) { return Colour (r, g, b); }
        static Colour fromRGBA (uint8 r, uint8 g, uint8 b, uint8 a) { return Colour (r, g, b, a); }
        static Colour fromHSV (float, float, float, float) { return {}; }
        Colour withAlpha (float) const { return *this; }
        Colour withAlpha (uint8) const { return *this; }
        Colour withMultipliedAlpha (float) const { return *this; }
        Colour withMultipliedBrightness (float) const { return *this; }
        Colour withMultipliedSaturation (float) const { return *this; }
        Colour withRotatedHue (float) const { return *this; }
        Colour withBrightness (float) const { return *this; }
        Colour withSaturation (float) const { return *this; }
        Colour brighter (float = 0.4f) const { return *this; }
        Colour darker (float = 0.4f) const { return *this; }
        Colour interpolatedWith (Colour, float) const { return *this; }
        Colour overlaidWith (Colour) const { return *this; }
        Colour contrasting (float = 0.5f) const { return *this; }
        uint8 getRed() const { return (uint8) (v >> 16); } uint8 getGreen() const { return (uint8) (v >> 8); }
        uint8 getBlue() const { return (uint8) v; } uint8 getAlpha() const { return (uint8) (v >> 24); }
        float getFloatRed() const { return getRed() / 255.0f; } float getFloatGreen() const { return getGreen() / 255.0f; }
        float getFloatBlue() const { return getBlue() / 255.0f; } float getFloatAlpha() const { return getAlpha() / 255.0f; }
        float getHue() const { return 0; } float getSaturation() const { return 0; } float getBrightness() const { return 0; }
        uint32 getARGB() const { return v; }
        bool operator== (const Colour& o) const { return v == o.v; }
        bool operator!= (const Colour& o) const { return v != o.v; }
    private:
        uint32 v = 0;
    };
    namespace Colours
    {
        inline const Colour black (0xff000000u), white (0xffffffffu), transparentBlack (0x00000000u), transparentWhite (0x00ffffffu),
                            grey (0xff808080u), red (0xffff0000u), green (0xff008000u), orange (0xffffa500u);
    }

    class ColourGradient
    {
    public:
        ColourGradient() = default;
        ColourGradient (Colour, float, float, Colour, float, float, bool) {}
        ColourGradient (Colour, Point<float>, Colour, Point<float>, bool) {}
        int addColour (double, Colour) { return 0; }
        void multiplyOpacity (float) {}
        static ColourGradient vertical (Colour, float, Colour, float) { return {}; }
        static ColourGradient horizontal (Colour, float, Colour, float) { return {}; }
    };

    class Path
    {
    public:
        void startNewSubPath (float, float) {} void startNewSubPath (Point<float>) {}
        void lineTo (float, float) {} void lineTo (Point<float>) {}
        void quadraticTo (float, float, float, float) {} void quadraticTo (Point<float>, Point<float>) {}
        void cubicTo (float, float, float, float, float, float) {} void cubicTo (Point<float>, Point<float>, Point<float>) {}
        void closeSubPath() {} void clear() {}
        void addEllipse (float, float, float, float) {} void addEllipse (Rectangle<float>) {}
        void addRectangle (float, float, float, float) {} void addRectangle (Rectangle<float>) {} void addRectangle (Rectangle<int>) {}
        void addRoundedRectangle (float, float, float, float, float) {}
        void addCentredArc (float, float, float, float, float, float, float, bool) {}
        void addArc (float, float, float, float, float, float, bool) {}
        void addPieSegment (float, float, float, float, float, float, float) {}
        void addStar (Point<float>, int, float, float, float = 0.0f) {}
        void addTriangle (float, float, float, float, float, float) {}
        void addPath (const Path&) {} void addPath (const Path&, const AffineTransform&) {}
        void applyTransform (const AffineTransform&) {}
        Rectangle<float> getBounds() const { return {}; }
        bool isEmpty() const { return true; }
        Path createPathWithRoundedCorners (float) const { return {}; }
    };
    struct PathStrokeType
    {
        enum JointStyle { mitered, curved, beveled }; enum EndCapStyle { butt, square, rounded };
        PathStrokeType (float, JointStyle = mitered, EndCapStyle = butt) {}
    };

    struct Line { float x1, y1, x2, y2; };

    struct Font
    {
        enum FontStyleFlags { plain = 0, bold = 1, italic = 2, underlined = 4 };
        Font() = default; explicit Font (const struct FontOptions&) {}
        Font withHeight (float) const { return *this; } Font boldened() const { return *this; }
        float getHeight() const { return 12; }
    };
    struct FontOptions
    {
        FontOptions() = default; FontOptions (float) {} FontOptions (float, int) {} FontOptions (const char*, float, int) {}
        FontOptions withHeight (float) const { return *this; } FontOptions withStyle (const char*) const { return *this; }
        FontOptions withName (const char*) const { return *this; }
    };

    class Graphics;
    class Image
    {
    public:
        enum PixelFormat { UnknownFormat, RGB, ARGB, SingleChannel };
        Image() = default;
        Image (PixelFormat f, int w, int h, bool clear) : fmt (f), width (w), height (h) { (void) clear; }
        bool isValid() const { return width > 0 && height > 0; }
        int getWidth() const { return width; } int getHeight() const { return height; }
        Rectangle<int> getBounds() const { return { 0, 0, width, height }; }
        Image rescaled (int w, int h, int /*Graphics::ResamplingQuality*/ = 1) const { return Image (fmt, w, h, true); }
        Image createCopy() const { return *this; }
        struct BitmapData
        {
            enum ReadWriteMode { readOnly, writeOnly, readWrite };
            BitmapData (Image&, ReadWriteMode) {}
            BitmapData (Image&, int, int, int, int, ReadWriteMode) {}
            void setPixelColour (int, int, Colour) const {}
            Colour getPixelColour (int, int) const { return {}; }
            uint8* data = nullptr; int width = 0, height = 0, pixelStride = 4, lineStride = 0;
        };
    private:
        PixelFormat fmt = UnknownFormat; int width = 0, height = 0;
    };
    struct ImageCache { static Image getFromMemory (const void*, int) { return Image (Image::ARGB, 8, 8, true); } };

    struct LowLevelGraphicsContext { float getPhysicalPixelScaleFactor() const { return 1.0f; } };
    struct RectanglePlacement { enum { stretchToFit = 0, centred = 1 }; RectanglePlacement (int = 0) {} };

    class Graphics
    {
    public:
        enum ResamplingQuality { lowResamplingQuality = 0, mediumResamplingQuality = 1, highResamplingQuality = 2 };
        explicit Graphics (Image&) {}
        void setColour (Colour) {} void setOpacity (float) {}
        void setGradientFill (const ColourGradient&) {}
        void fillAll() {} void fillAll (Colour) {}
        void fillRect (int, int, int, int) {} void fillRect (float, float, float, float) {}
        void fillRect (Rectangle<int>) {} void fillRect (Rectangle<float>) {}
        void drawRect (int, int, int, int, int = 1) {} void drawRect (Rectangle<int>, int = 1) {} void drawRect (float, float, float, float, float = 1.0f) {}
        void fillEllipse (float, float, float, float) {} void fillEllipse (Rectangle<float>) {}
        void drawEllipse (float, float, float, float, float) {} void drawEllipse (Rectangle<float>, float) {}
        void fillRoundedRectangle (float, float, float, float, float) {} void fillRoundedRectangle (Rectangle<float>, float) {}
        void drawRoundedRectangle (float, float, float, float, float, float) {} void drawRoundedRectangle (Rectangle<float>, float, float) {}
        void drawLine (float, float, float, float) {} void drawLine (float, float, float, float, float) {} void drawLine (Line) {} void drawLine (Line, float) {}
        void fillPath (const Path&) {} void fillPath (const Path&, const AffineTransform&) {}
        void strokePath (const Path&, const PathStrokeType&) {} void strokePath (const Path&, const PathStrokeType&, const AffineTransform&) {}
        void setFont (const Font&) {} void setFont (float) {}
        void drawText (const String&, int, int, int, int, Justification, bool = true) {}
        void drawText (const String&, Rectangle<int>, Justification, bool = true) {}
        void drawText (const String&, Rectangle<float>, Justification, bool = true) {}
        void drawFittedText (const String&, int, int, int, int, Justification, int) {}
        void drawFittedText (const String&, Rectangle<int>, Justification, int) {}
        void drawImage (const Image&, Rectangle<float>, int = 0, bool = false) {}
        void drawImage (const Image&, int, int, int, int, int, int, int, int, bool = false) {}
        void drawImageAt (const Image&, int, int, bool = false) {}
        void drawImageTransformed (const Image&, const AffineTransform&, bool = false) {}
        void drawImageWithin (const Image&, int, int, int, int, RectanglePlacement, bool = false) {}
        void setImageResamplingQuality (ResamplingQuality) {}
        void saveState() {} void restoreState() {}
        struct ScopedSaveState { ScopedSaveState (Graphics&) {} };
        bool reduceClipRegion (int, int, int, int) { return true; } bool reduceClipRegion (Rectangle<int>) { return true; }
        bool reduceClipRegion (const Path&, const AffineTransform& = AffineTransform()) { return true; }
        void excludeClipRegion (Rectangle<int>) {}
        Rectangle<int> getClipBounds() const { return {}; }
        bool clipRegionIntersects (Rectangle<int>) const { return true; }
        void addTransform (const AffineTransform&) {}
        LowLevelGraphicsContext& getInternalContext() { return ctx; }
    private:
        LowLevelGraphicsContext ctx;
    };

    struct MouseCursor { enum StandardCursorType { NormalCursor, PointingHandCursor }; MouseCursor (StandardCursorType = NormalCursor) {} };
    class LookAndFeel { public: virtual ~LookAndFeel() = default; void setColour (int, Colour) {} };

    class Component
    {
    public:
        virtual ~Component() = default;
        virtual void paint (Graphics&) {}
        virtual void paintOverChildren (Graphics&) {}
        virtual void resized() {}
        void addAndMakeVisible (Component&) {} void addChildComponent (Component&) {}
        void setBounds (int, int, int, int) {} void setBounds (Rectangle<int>) {}
        Rectangle<int> getLocalBounds() const { return { 0, 0, mw, mh }; }
        Rectangle<int> getBounds() const { return { mx, my, mw, mh }; }
        int getX() const { return mx; } int getY() const { return my; } int getWidth() const { return mw; } int getHeight() const { return mh; }
        void repaint() {} void repaint (int, int, int, int) {} void repaint (Rectangle<int>) {}
        void setMouseCursor (const MouseCursor&) {} void setName (const String&) {} String getName() const { return {}; }
        void setLookAndFeel (LookAndFeel*) {} void setInterceptsMouseClicks (bool, bool) {} void setOpaque (bool) {}
        void setWantsKeyboardFocus (bool) {} void setBufferedToImage (bool) {} void setSize (int, int) {}
        bool isMouseButtonDown() const { return false; } bool isMouseOver (bool = false) const { return false; }
        void setColour (int, Colour) {}
        Rectangle<int> getBoundsInParent() const { return getBounds(); }
    private:
        int mx = 0, my = 0, mw = 0, mh = 0;
    };
    class Timer { public: virtual ~Timer() = default; void startTimerHz (int) {} void startTimer (int) {} void stopTimer() {} virtual void timerCallback() = 0; };

    struct TooltipWindow { TooltipWindow (Component*, int) {} };
    struct Random { Random (int64) {} Random() {} float nextFloat() { return 0.5f; } int nextInt (int n) { return n / 2; } int nextInt() { return 0; } bool nextBool() { return false; } };
    struct Time { static double getMillisecondCounterHiRes() { return 0.0; } static uint32 getMillisecondCounter() { return 0; } };

    class Label : public Component
    {
    public:
        enum ColourIds { backgroundColourId = 0x1000280, textColourId = 0x1000281 };
        void setFont (const Font&) {} void setJustificationType (Justification) {} void setText (const String&, NotificationType) {}
    };
    class SettableTooltipClient { public: void setTooltip (const String&) {} };
    class Slider : public Component, public SettableTooltipClient
    {
    public:
        enum SliderStyle { LinearHorizontal, LinearVertical, Rotary, RotaryHorizontalVerticalDrag, RotaryVerticalDrag };
        enum TextEntryBoxPosition { NoTextBox, TextBoxLeft, TextBoxRight, TextBoxAbove, TextBoxBelow };
        enum ColourIds { backgroundColourId = 0x1001200, thumbColourId, trackColourId, rotarySliderFillColourId, textBoxTextColourId = 0x1001700, textBoxBackgroundColourId, textBoxOutlineColourId };
        void setSliderStyle (SliderStyle) {} void setTextBoxStyle (TextEntryBoxPosition, bool, int, int) {}
        void setRange (double, double, double = 0.0) {} double getValue() const { return 0; } void setValue (double, NotificationType = sendNotificationAsync) {}
        void setDoubleClickReturnValue (bool, double) {} void setMouseDragSensitivity (int) {} void setNumDecimalPlacesToDisplay (int) {}
        void setRotaryParameters (float, float, bool) {} double valueToProportionOfLength (double) const { return 0; }
        double getMinimum() const { return 0; } double getMaximum() const { return 1; }
        std::function<void()> onValueChange;
    };
    class Button : public Component, public SettableTooltipClient
    {
    public:
        virtual ~Button() override = default;
        enum ConnectedEdgeFlags { ConnectedOnLeft = 1 };
        void setButtonText (const String&) {} String getButtonText() const { return {}; }
        void setClickingTogglesState (bool) {} void setToggleState (bool, NotificationType) {} bool getToggleState() const { return false; }
        std::function<void()> onClick;
    };
    class TextButton : public Button { public: enum ColourIds { buttonColourId = 0x1000100, buttonOnColourId, textColourOffId = 0x1000102, textColourOnId }; };
    class PopupMenu { public: enum ColourIds { backgroundColourId = 0x1000700, textColourId, headerTextColourId, highlightedBackgroundColourId, highlightedTextColourId }; };
    class ComboBox : public Component, public SettableTooltipClient
    {
    public:
        enum ColourIds { backgroundColourId = 0x1000b00, textColourId, outlineColourId, buttonColourId, arrowColourId };
        void addItem (const String&, int) {} void addSectionHeading (const String&) {} void addSeparator() {}
        int getSelectedId() const { return 1; } void setSelectedId (int, NotificationType = sendNotificationAsync) {}
        void setJustificationType (Justification) {} int getNumItems() const { return 0; }
        std::function<void()> onChange;
    };
    class LookAndFeel_V4 : public LookAndFeel
    {
    public:
        virtual void drawRotarySlider (Graphics&, int, int, int, int, float, float, float, Slider&) {}
        virtual void drawComboBox (Graphics&, int, int, bool, int, int, int, int, ComboBox&) {}
        virtual void positionComboBoxText (ComboBox&, Label&) {}
        virtual void drawButtonBackground (Graphics&, Button&, const Colour&, bool, bool) {}
        virtual void drawButtonText (Graphics&, TextButton&, bool, bool) {}
        virtual void drawLinearSlider (Graphics&, int, int, int, int, float, float, float, Slider::SliderStyle, Slider&) {}
    };

    class AudioProcessorEditor : public Component
    {
    public:
        explicit AudioProcessorEditor (AudioProcessor&) {} explicit AudioProcessorEditor (AudioProcessor*) {}
        void setResizable (bool, bool) {} void setResizeLimits (int, int, int, int) {}
        AudioProcessor* ownerProcessor = nullptr;
    };

    // Attachments: constructed with (state, parameterID, control)
    struct SliderAttachmentMock { SliderAttachmentMock (AudioProcessorValueTreeState&, const String&, Slider&) {} };
}
