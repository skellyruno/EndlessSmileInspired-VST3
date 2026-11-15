// Minimal stand-in for the JUCE pieces used by PluginProcessor.cpp, so the real
// processBlock() can be run offline. The Reverb is a port of JUCE's Freeverb and the
// filters/delay follow JUCE's TPT/linear implementations. NOT part of the plugin build.
#pragma once
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#if defined(__SSE__)
 #include <xmmintrin.h>
 #include <pmmintrin.h>
#endif

#define JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(x)
#define JUCE_CALLTYPE

namespace juce
{
    using uint32 = std::uint32_t;
    template <class T> using Optional = std::optional<T>;

    template <class T> constexpr T jmin (T a, T b) { return a < b ? a : b; }
    template <class T> constexpr T jmin (T a, T b, T c) { return jmin (jmin (a, b), c); }
    template <class T> constexpr T jmax (T a, T b) { return a > b ? a : b; }
    template <class T> constexpr T jlimit (T lo, T hi, T v) { return v < lo ? lo : (v > hi ? hi : v); }
    template <class T> constexpr T jmap (T v, T lo, T hi) { return lo + v * (hi - lo); }
    inline int roundToInt (float v) { return (int) std::floor (v + 0.5f); }
    template <class... A> void ignoreUnused (A&&...) {}
    template <class T> struct MathConstants { static constexpr T twoPi = (T) 6.283185307179586; static constexpr T pi = (T) 3.141592653589793; static constexpr T halfPi = (T) 1.5707963267948966; };

    struct ScopedNoDenormals
    {
        ScopedNoDenormals() { _mm_setcsr (_mm_getcsr() | 0x8040); }   // FTZ + DAZ
    };

    struct String : std::string { String() = default; String (double v, int) : std::string (std::to_string (v)) {} String (int v) : std::string (std::to_string (v)) {} String (const char* s) : std::string (s) {} String (const std::string& s) : std::string (s) {} };
    struct StringArray { std::vector<std::string> items; int size() const { return (int) items.size(); } const std::string& operator[] (int i) const { return items[(size_t) i]; } StringArray (std::initializer_list<const char*> l) { for (auto* s : l) items.push_back (s); } };
    struct MemoryBlock {};
    struct XmlElement { String tag; bool hasTagName (const String& t) const { return tag == t; } };
    struct var
    {
        var() = default; var (bool b) : v (b ? 1 : 0) {} var (int i) : v (i) {} var (double d) : v (d) {}
        operator bool() const { return v != 0; } operator int() const { return (int) v; } operator double() const { return v; }
        double v = 0;
    };
    struct Identifier : String { Identifier() = default; Identifier (const char* s) : String (s) {} };
    struct UndoManager {};
    struct ValueTree
    {
        var getProperty (const Identifier&, const var& def = var()) const { return def; }
        ValueTree& setProperty (const Identifier&, const var&, UndoManager*) { return *this; }
        String type { "PARAMETERS" };
        String getType() const { return type; }
        std::unique_ptr<XmlElement> createXml() const { return std::make_unique<XmlElement>(); }
        static ValueTree fromXml (const XmlElement&) { return {}; }
    };

    template <class T> class AudioBuffer
    {
    public:
        AudioBuffer (int ch, int n) : data ((size_t) ch, std::vector<T> ((size_t) n, (T) 0)) {}
        int getNumChannels() const { return (int) data.size(); }
        int getNumSamples() const { return data.empty() ? 0 : (int) data[0].size(); }
        T getSample (int c, int i) const { return data[(size_t) c][(size_t) i]; }
        void setSample (int c, int i, T v) { data[(size_t) c][(size_t) i] = v; }
        void clear (int c, int start, int n) { for (int i = 0; i < n; ++i) data[(size_t) c][(size_t) (start + i)] = 0; }
        T* getWritePointer (int c) { return data[(size_t) c].data(); }
    private:
        std::vector<std::vector<T>> data;
    };
    struct MidiBuffer {};

    struct AudioChannelSet
    {
        int n = 0;
        static AudioChannelSet mono() { return { 1 }; }
        static AudioChannelSet stereo() { return { 2 }; }
        bool operator== (const AudioChannelSet& o) const { return n == o.n; }
    };

    struct AudioPlayHead
    {
        struct PositionInfo
        {
            Optional<double> bpm, ppq; bool playing = false;
            Optional<double> getBpm() const { return bpm; }
            Optional<double> getPpqPosition() const { return ppq; }
            bool getIsPlaying() const { return playing; }
        };
        PositionInfo info;
        Optional<PositionInfo> getPosition() const { return info; }
    };

#ifdef MOCK_UI
    class AudioProcessorEditor;
#else
    class AudioProcessorEditor { public: virtual ~AudioProcessorEditor() = default; };
#endif

    class AudioProcessor
    {
    public:
        struct BusesProperties
        {
            BusesProperties& withInput (const char*, AudioChannelSet, bool) { return *this; }
            BusesProperties& withOutput (const char*, AudioChannelSet, bool) { return *this; }
        };
        struct BusesLayout
        {
            AudioChannelSet in, out;
            AudioChannelSet getMainInputChannelSet() const { return in; }
            AudioChannelSet getMainOutputChannelSet() const { return out; }
        };
        AudioProcessor (const BusesProperties&) {}
        virtual ~AudioProcessor() = default;
        virtual void prepareToPlay (double, int) = 0;
        virtual void releaseResources() = 0;
        virtual bool isBusesLayoutSupported (const BusesLayout&) const = 0;
        virtual void processBlock (AudioBuffer<float>&, MidiBuffer&) = 0;
        virtual void processBlock (AudioBuffer<double>&, MidiBuffer&) {}
        virtual AudioProcessorEditor* createEditor() = 0;
        virtual bool hasEditor() const = 0;
        virtual const String getName() const = 0;
        virtual bool acceptsMidi() const = 0;
        virtual bool producesMidi() const = 0;
        virtual bool isMidiEffect() const = 0;
        virtual double getTailLengthSeconds() const = 0;
        virtual int getNumPrograms() = 0;
        virtual int getCurrentProgram() = 0;
        virtual void setCurrentProgram (int) = 0;
        virtual const String getProgramName (int) = 0;
        virtual void changeProgramName (int, const String&) = 0;
        virtual void getStateInformation (MemoryBlock&) = 0;
        virtual void setStateInformation (const void*, int) = 0;
        static void copyXmlToBinary (const XmlElement&, MemoryBlock&) {}
        static std::unique_ptr<XmlElement> getXmlFromBinary (const void*, int) { return nullptr; }
        AudioPlayHead* getPlayHead() const { return mockPlayHead; }
        int getTotalNumInputChannels() const { return channelsIn; }
        int getTotalNumOutputChannels() const { return channelsOut; }
        int getBlockSize() const { return blockSize; }
        AudioPlayHead* mockPlayHead = nullptr;
        int channelsIn = 2, channelsOut = 2, blockSize = 512;
    };

    struct ParameterID { ParameterID (const char* s, int) : id (s) {} std::string id; };
    template <class T> struct NormalisableRange { NormalisableRange (T, T, T) {} };
    struct RangedAudioParameter { virtual ~RangedAudioParameter() = default; std::string id; float def = 0; };
    struct AudioParameterFloat : RangedAudioParameter { AudioParameterFloat (ParameterID i, const char*, NormalisableRange<float>, float d) { id = i.id; def = d; } };
    struct AudioParameterChoice : RangedAudioParameter { AudioParameterChoice (ParameterID i, const char*, StringArray, int d) { id = i.id; def = (float) d; } };
    struct AudioParameterBool : RangedAudioParameter { AudioParameterBool (ParameterID i, const char*, bool d) { id = i.id; def = d ? 1.0f : 0.0f; } };

#ifdef MOCK_UI
    class Slider; class ComboBox; class Button;
#endif
    class AudioProcessorValueTreeState
    {
    public:
#ifdef MOCK_UI
        template <class C> struct Attachment { Attachment (AudioProcessorValueTreeState&, const String&, C&) {} };
        using SliderAttachment = Attachment<Slider>;
        using ComboBoxAttachment = Attachment<ComboBox>;
        using ButtonAttachment = Attachment<Button>;
#endif
        struct ParameterLayout
        {
            std::vector<std::unique_ptr<RangedAudioParameter>> params;
            template <class It> ParameterLayout (It b, It e) { for (; b != e; ++b) params.push_back (std::move (*b)); }
        };
        AudioProcessorValueTreeState (AudioProcessor&, void*, const char*, ParameterLayout layout)
        {
            for (auto& p : layout.params) { values[p->id].store (p->def); }
        }
        std::atomic<float>* getRawParameterValue (const String& id) { return &values[id]; }
        ValueTree copyState() { return state; }
        void replaceState (const ValueTree& v) { state = v; }
        ValueTree state;
    private:
        std::map<std::string, std::atomic<float>> values;
    };

    template <class T> class SmoothedValue
    {
    public:
        void reset (double sr, double seconds) { steps = (int) std::floor (sr * seconds); setCurrentAndTargetValue (target); }
        void setCurrentAndTargetValue (T v) { current = target = v; countdown = 0; }
        void setTargetValue (T v)
        {
            if (v == target) return;
            target = v;
            if (steps <= 0) { current = v; return; }
            countdown = steps; step = (target - current) / (T) steps;
        }
        T getNextValue() { if (countdown <= 0) return target; --countdown; current += step; return countdown > 0 ? current : target; }
        T getCurrentValue() const { return current; }
    private:
        T current {}, target {}, step {}; int steps = 0, countdown = 0;
    };

    class Reverb   // port of JUCE's Freeverb-based juce::Reverb
    {
    public:
        struct Parameters { float roomSize = 0.5f, damping = 0.5f, wetLevel = 0.33f, dryLevel = 0.4f, width = 1.0f, freezeMode = 0.0f; };
        Reverb() { setParameters (Parameters()); setSampleRate (44100.0); }
        void setSampleRate (double sr)
        {
            static const short combT[] { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
            static const short apT[] { 556, 441, 341, 225 };
            const int isr = (int) sr;
            for (int i = 0; i < 8; ++i) { comb[0][i].setSize ((isr * combT[i]) / 44100); comb[1][i].setSize ((isr * (combT[i] + 23)) / 44100); }
            for (int i = 0; i < 4; ++i) { ap[0][i].setSize ((isr * apT[i]) / 44100); ap[1][i].setSize ((isr * (apT[i] + 23)) / 44100); }
            const double t = 0.01;
            damping.reset (sr, t); feedback.reset (sr, t); dryGain.reset (sr, t); wetGain1.reset (sr, t); wetGain2.reset (sr, t);
        }
        void setParameters (const Parameters& p)
        {
            const float wet = p.wetLevel * 3.0f;
            dryGain.setTargetValue (p.dryLevel * 2.0f);
            wetGain1.setTargetValue (0.5f * wet * (1.0f + p.width));
            wetGain2.setTargetValue (0.5f * wet * (1.0f - p.width));
            gain = 0.015f; params = p;
            damping.setTargetValue (p.damping * 0.4f);
            feedback.setTargetValue (p.roomSize * 0.28f + 0.7f);
        }
        const Parameters& getParameters() const { return params; }
        void reset() { for (auto& ch : comb) for (auto& c : ch) c.clear(); for (auto& ch : ap) for (auto& a : ch) a.clear();
                       damping.setCurrentAndTargetValue (damping.getCurrentValue()); }
        void processStereo (float* l, float* r, int n)
        {
            for (int i = 0; i < n; ++i)
            {
                const float in = (l[i] + r[i]) * gain, d = damping.getNextValue(), fb = feedback.getNextValue();
                float oL = 0, oR = 0;
                for (int j = 0; j < 8; ++j) { oL += comb[0][j].process (in, d, fb); oR += comb[1][j].process (in, d, fb); }
                for (int j = 0; j < 4; ++j) { oL = ap[0][j].process (oL); oR = ap[1][j].process (oR); }
                const float dry = dryGain.getNextValue(), w1 = wetGain1.getNextValue(), w2 = wetGain2.getNextValue();
                l[i] = oL * w1 + oR * w2 + l[i] * dry;
                r[i] = oR * w1 + oL * w2 + r[i] * dry;
            }
        }
        void processMono (float* s, int n)
        {
            for (int i = 0; i < n; ++i)
            {
                const float in = s[i] * gain, d = damping.getNextValue(), fb = feedback.getNextValue();
                float o = 0;
                for (int j = 0; j < 8; ++j) o += comb[0][j].process (in, d, fb);
                for (int j = 0; j < 4; ++j) o = ap[0][j].process (o);
                const float dry = dryGain.getNextValue(), w1 = wetGain1.getNextValue(), w2 = wetGain2.getNextValue();
                s[i] = o * w1 + o * w2 + s[i] * dry;
            }
        }
    private:
        struct Comb
        {
            std::vector<float> buf; int idx = 0; float last = 0;
            void setSize (int n) { buf.assign ((size_t) std::max (1, n), 0.0f); idx = 0; last = 0; }
            void clear() { std::fill (buf.begin(), buf.end(), 0.0f); last = 0; }
            float process (float in, float damp, float fb)
            {
                const float out = buf[(size_t) idx];
                last = out * (1.0f - damp) + last * damp;
                buf[(size_t) idx] = in + last * fb;
                if (++idx >= (int) buf.size()) idx = 0;
                return out;
            }
        };
        struct AllPass
        {
            std::vector<float> buf; int idx = 0;
            void setSize (int n) { buf.assign ((size_t) std::max (1, n), 0.0f); idx = 0; }
            void clear() { std::fill (buf.begin(), buf.end(), 0.0f); }
            float process (float in)
            {
                const float out = buf[(size_t) idx];
                buf[(size_t) idx] = in + out * 0.5f;
                if (++idx >= (int) buf.size()) idx = 0;
                return out - in;
            }
        };
        Parameters params; float gain = 0.015f;
        Comb comb[2][8]; AllPass ap[2][4];
        SmoothedValue<float> damping, feedback, dryGain, wetGain1, wetGain2;
    };

    namespace dsp
    {
        struct ProcessSpec { double sampleRate; uint32 maximumBlockSize; uint32 numChannels; };
        enum class StateVariableTPTFilterType { lowpass, bandpass, highpass };
        template <class T> class StateVariableTPTFilter
        {
        public:
            void prepare (const ProcessSpec& s) { sr = s.sampleRate; update(); }
            void reset() { s1 = s2 = 0; }
            void setType (StateVariableTPTFilterType t) { type = t; }
            void setCutoffFrequency (T f) { cutoff = f; update(); }
            void setResonance (T q) { res = q; update(); }
            T processSample (int, T x)
            {
                const T yHP = h * (x - s1 * (g + R2) - s2);
                const T yBP = yHP * g + s1; s1 = yHP * g + yBP;
                const T yLP = yBP * g + s2; s2 = yBP * g + yLP;
                return type == StateVariableTPTFilterType::lowpass ? yLP : (type == StateVariableTPTFilterType::bandpass ? yBP : yHP);
            }
        private:
            void update() { g = std::tan (3.14159265358979f * cutoff / (T) sr); R2 = 1 / res; h = 1 / (1 + R2 * g + g * g); }
            StateVariableTPTFilterType type = StateVariableTPTFilterType::lowpass;
            double sr = 44100; T cutoff = 1000, res = (T) 0.7071, g = 0, R2 = 1, h = 1, s1 = 0, s2 = 0;
        };
        namespace DelayLineInterpolationTypes { struct Linear {}; }
        template <class T, class I> class DelayLine
        {
        public:
            void setMaximumDelayInSamples (int n) { buf.assign ((size_t) n + 2, 0); }
            void prepare (const ProcessSpec&) {}
            void reset() { std::fill (buf.begin(), buf.end(), (T) 0); }
            void setDelay (T d) { delay = d; }
            void pushSample (int, T x) { wp = (wp + (int) buf.size() - 1) % (int) buf.size(); buf[(size_t) wp] = x; }
            T popSample (int)
            {
                const int n = (int) buf.size(); const int di = (int) delay; const T f = delay - (T) di;
                const T a = buf[(size_t) ((wp + di) % n)], b = buf[(size_t) ((wp + di + 1) % n)];
                return a + f * (b - a);
            }
        private:
            std::vector<T> buf; int wp = 0; T delay = 0;
        };
    }
}
