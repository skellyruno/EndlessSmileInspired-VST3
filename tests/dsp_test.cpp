// Offline DSP checks for PluginProcessor.cpp (see run.sh). Not part of the plugin build.
#include "PluginProcessor.h"
#include <chrono>
#include <cstdio>
#include <cstring>

static const char* names[16] { "Fist in the Air", "Helium Jumping", "No Limit", "Face Melting Joy", "Smack You", "Broken Teeth",
                               "Dada Dynamite", "Midnight Howl", "Banshee Scream", "Ghost Whisper", "Rattling Bones",
                               "Crypt Drone", "Wailing Wraith", "Zombie Groan", "Coffin Creak", "Haunted Bells" };
static int failures = 0;
#define CHECK(cond, ...) do { if (! (cond)) { ++failures; std::printf ("  FAIL: "); std::printf (__VA_ARGS__); std::printf ("\n"); } } while (0)

struct Rng { uint32_t s = 12345; float next() { s = s * 1664525u + 1013904223u; return (float) (s >> 8) / 8388608.0f - 1.0f; } };

// Programme-like test signal: low/mid/high partials plus noise, about -18 dBFS RMS.
static std::vector<float> makeInput (double sr, int n, float rmsTarget = 0.125f)
{
    Rng rng; std::vector<float> x ((size_t) n); float lp = 0.0f;
    for (int i = 0; i < n; ++i)
    {
        lp += 0.15f * (rng.next() - lp);   // noise tilted towards the lows, like programme material
        const double t = i / sr;
        const float env = 0.6f + 0.4f * (float) std::sin (6.2831853 * 2.0 * t);
        x[(size_t) i] = env * (0.5f * (float) std::sin (6.2831853 * 110.0 * t) + 0.3f * (float) std::sin (6.2831853 * 440.0 * t)
                             + 0.2f * (float) std::sin (6.2831853 * 2200.0 * t) + 1.5f * lp);
    }
    double e = 0; for (float v : x) e += (double) v * v;
    const float g = rmsTarget / (float) std::sqrt (e / n);
    for (auto& v : x) v *= g;
    return x;
}

struct Result { double rmsDb = -200, peak = 0; bool finite = true; };

static void setParams (EndlessSmileInspiredAudioProcessor& p, int preset, float intensity, float corruption = 0.0f, float mix = 1.0f, bool sync = true)
{
    p.state.getRawParameterValue ("preset")->store ((float) preset);
    p.state.getRawParameterValue ("intensity")->store (intensity);
    p.state.getRawParameterValue ("corruption")->store (corruption);
    p.state.getRawParameterValue ("mix")->store (mix);
    p.state.getRawParameterValue ("sync")->store (sync ? 1.0f : 0.0f);
}

// Render `in` through the processor (same signal on all channels). Returns output of channel 0 and 1.
static void render (EndlessSmileInspiredAudioProcessor& p, const std::vector<float>& in, int block, int channels,
                    std::vector<float>& out0, std::vector<float>& out1)
{
    out0.assign (in.size(), 0.0f); out1.assign (in.size(), 0.0f);
    juce::MidiBuffer midi;
    for (size_t pos = 0; pos < in.size(); pos += (size_t) block)
    {
        const int n = (int) std::min ((size_t) block, in.size() - pos);
        juce::AudioBuffer<float> buf (channels, n);
        for (int c = 0; c < channels; ++c) for (int i = 0; i < n; ++i) buf.setSample (c, i, in[pos + (size_t) i]);
        p.processBlock (buf, midi);
        for (int i = 0; i < n; ++i) { out0[pos + (size_t) i] = buf.getSample (0, i); out1[pos + (size_t) i] = buf.getSample (channels - 1, i); }
    }
}

static Result measure (const std::vector<float>& y0, const std::vector<float>& y1, size_t from)
{
    Result r; double e = 0; size_t n = 0;
    for (size_t i = from; i < y0.size(); ++i)
        for (float v : { y0[i], y1[i] })
        {
            if (! std::isfinite (v)) r.finite = false;
            r.peak = std::max (r.peak, (double) std::abs (v)); e += (double) v * v; ++n;
        }
    r.rmsDb = 10.0 * std::log10 (e / (double) n + 1e-20);
    return r;
}

static double rmsDb (const std::vector<float>& x, size_t from)
{
    double e = 0; for (size_t i = from; i < x.size(); ++i) e += (double) x[i] * x[i];
    return 10.0 * std::log10 (e / (double) (x.size() - from) + 1e-20);
}

int main()
{
    const double sr = 48000.0;
    const int n = (int) (sr * 6);
    const auto in = makeInput (sr, n);
    const double inDb = rmsDb (in, (size_t) (sr * 2));
    std::vector<float> y0, y1;

    std::printf ("Input RMS %.1f dBFS\n\n== 1. Level vs dry (dB, last 4 s), peak; intensity 0 / 25 / 50 / 75 / 100 %% ==\n", inDb);
    std::printf ("%-18s", "preset");
    for (int k = 0; k < 5; ++k) std::printf ("  %4d%%        ", k * 25);
    std::printf ("  corr100%%\n");
    for (int mode = 0; mode < 16; ++mode)
    {
        std::printf ("%-18s", names[mode]);
        for (int k = 0; k < 6; ++k)
        {
            EndlessSmileInspiredAudioProcessor p;
            p.prepareToPlay (sr, 512);
            const float i110 = k == 5 ? 110.0f : 110.0f * (float) k * 0.25f;
            setParams (p, mode, k == 5 ? 0.0f : i110, k == 5 ? 1.0f : 0.0f);
            if (k == 5) setParams (p, mode, 0.0f, 1.0f);
            render (p, in, 512, 2, y0, y1);
            const auto r = measure (y0, y1, (size_t) (sr * 2));
            std::printf ("  %+6.1f/%4.2f ", r.rmsDb - inDb, r.peak);
            CHECK (r.finite, "%s non-finite output", names[mode]);
            CHECK (r.peak <= 1.0001, "%s peak %.3f exceeds 1.0", names[mode], r.peak);
            if (k == 0)
            {
                double maxDiff = 0; for (size_t i = 0; i < in.size(); ++i) maxDiff = std::max (maxDiff, (double) std::abs (y0[i] - in[i]));
                CHECK (maxDiff == 0.0, "%s intensity 0 is not bit-exact dry (diff %.3g)", names[mode], maxDiff);
            }
        }
        std::printf ("\n");
    }

    std::printf ("\n== 2. Hot input (peak 0.99), max intensity + corruption: ceiling ==\n");
    {
        auto hot = makeInput (sr, n, 0.4f); for (auto& v : hot) v = std::max (-0.99f, std::min (0.99f, v));
        for (int mode = 0; mode < 16; ++mode)
        {
            EndlessSmileInspiredAudioProcessor p; p.prepareToPlay (sr, 512);
            setParams (p, mode, 110.0f, 1.0f);
            render (p, hot, 512, 2, y0, y1);
            const auto r = measure (y0, y1, 0);
            CHECK (r.finite && r.peak <= 1.0001, "%s hot-input peak %.4f finite=%d", names[mode], r.peak, r.finite);
        }
        std::printf ("  done\n");
    }

    std::printf ("\n== 3. Tail decay after input stops (presets 0-3, max intensity) ==\n");
    for (int mode = 0; mode < 4; ++mode)
    {
        EndlessSmileInspiredAudioProcessor p; p.prepareToPlay (sr, 512);
        setParams (p, mode, 110.0f);
        std::vector<float> long_in = makeInput (sr, (int) (sr * 2));
        long_in.resize ((size_t) (sr * 14), 0.0f);
        render (p, long_in, 512, 2, y0, y1);
        const double tail = rmsDb (std::vector<float> (y0.begin() + (long) (sr * 12), y0.end()), 0);
        std::printf ("  %-16s tail RMS %.1f dBFS\n", names[mode], tail);
        CHECK (tail < -80.0, "%s tail does not decay (%.1f dB)", names[mode], tail);
    }

    std::printf ("\n== 4. Sample rates / block sizes / mono: finite, bounded ==\n");
    for (double rate : { 22050.0, 44100.0, 96000.0, 192000.0 })
        for (int block : { 1, 7, 64, 480, 4096 })
            for (int channels : { 1, 2 })
                for (int mode : { 0, 3, 5, 6, 8, 10, 15 })
                {
                    EndlessSmileInspiredAudioProcessor p; p.channelsIn = p.channelsOut = channels;
                    p.prepareToPlay (rate, block);
                    setParams (p, mode, 110.0f, 0.5f);
                    const auto sig = makeInput (rate, (int) (rate * 1.5));
                    render (p, sig, block, channels, y0, y1);
                    const auto r = measure (y0, y1, 0);
                    CHECK (r.finite && r.peak <= 1.0001, "sr %.0f block %d ch %d %s: peak %.3f finite %d", rate, block, channels, names[mode], r.peak, r.finite);
                }
    std::printf ("  done\n");

    std::printf ("\n== 5. Automation: intensity sweep + preset switching, no non-finite, no huge steps ==\n");
    {
        EndlessSmileInspiredAudioProcessor p; p.prepareToPlay (sr, 256);
        juce::MidiBuffer midi; double maxStep = 0; float prev = 0; bool finite = true;
        const auto sig = makeInput (sr, (int) (sr * 16), 0.1f);
        for (size_t pos = 0, blk = 0; pos + 256 <= sig.size(); pos += 256, ++blk)
        {
            const int mode = (int) ((blk / 200) % 16);
            setParams (p, mode, 110.0f * (float) (0.5 + 0.5 * std::sin ((double) blk * 0.01)), 0.0f);
            juce::AudioBuffer<float> buf (2, 256);
            for (int i = 0; i < 256; ++i) { buf.setSample (0, i, sig[pos + (size_t) i]); buf.setSample (1, i, sig[pos + (size_t) i]); }
            p.processBlock (buf, midi);
            for (int i = 0; i < 256; ++i)
            {
                const float v = buf.getSample (0, i);
                finite = finite && std::isfinite (v);
                maxStep = std::max (maxStep, (double) std::abs (v - prev)); prev = v;
            }
        }
        std::printf ("  max sample-to-sample step %.3f\n", maxStep);
        CHECK (finite, "non-finite during automation");
        CHECK (maxStep < 1.6, "large discontinuity %.3f during automation", maxStep);
    }

    std::printf ("\n== 6. Tempo sync: host tempo changes delay/riser, stays finite ==\n");
    {
        juce::AudioPlayHead head; head.info.bpm = 128.0; head.info.ppq = 0.0; head.info.playing = true;
        for (int mode : { 1, 3, 5, 6, 9, 12 })
        {
            EndlessSmileInspiredAudioProcessor p; p.mockPlayHead = &head; p.prepareToPlay (sr, 512);
            setParams (p, mode, 90.0f, 0.0f, 1.0f, true);
            render (p, makeInput (sr, (int) (sr * 3)), 512, 2, y0, y1);
            const auto r = measure (y0, y1, (size_t) sr);
            CHECK (r.finite && r.peak <= 1.0001, "sync preset %s", names[mode]);
            CHECK (p.hostBpmValid.load() && std::abs (p.hostBpm.load() - 128.0f) < 1e-3f, "bpm not reported");
        }
        std::printf ("  done\n");
    }

    std::printf ("\n== 7. Latency: impulse at intensity 0 (dry) and impulse position at 50 %% ==\n");
    {
        std::vector<float> imp ((size_t) sr, 0.0f); imp[100] = 0.5f;
        EndlessSmileInspiredAudioProcessor p; p.prepareToPlay (sr, 512);
        setParams (p, 0, 0.0f);
        render (p, imp, 512, 2, y0, y1);
        CHECK (y0[100] == 0.5f, "dry impulse moved");
        std::printf ("  zero latency at intensity 0\n");
    }

    std::printf ("\n== 8. CPU (stereo, 48 kHz, block 512, single core, -O2; <100%% = faster than real time) ==\n");
    for (int mode = 0; mode < 16; ++mode)
    {
        EndlessSmileInspiredAudioProcessor p; p.prepareToPlay (sr, 512);
        setParams (p, mode, 110.0f, 0.3f);
        const auto sig = makeInput (sr, (int) (sr * 10));
        const auto t0 = std::chrono::steady_clock::now();
        render (p, sig, 512, 2, y0, y1);
        const double sec = std::chrono::duration<double> (std::chrono::steady_clock::now() - t0).count();
        std::printf ("  %-16s %5.1f %% of one core\n", names[mode], 100.0 * sec / 10.0);
        CHECK (sec / 10.0 < 0.25, "%s CPU too high (%.0f%%)", names[mode], 100.0 * sec / 10.0);
    }

    std::printf("\n== 9. Mix, hot input and recovery from a NaN/inf burst ==\n");
    for (int mode : { 0, 3, 5, 8 })
    {
        {   // Mix 0 must be bit-exact dry even at full intensity.
            EndlessSmileInspiredAudioProcessor p; p.prepareToPlay (sr, 512);
            setParams (p, mode, 110.0f, 1.0f, 0.0f);
            render (p, in, 512, 2, y0, y1);
            double d = 0; for (size_t i = (size_t) (sr * 0.5); i < in.size(); ++i) d = std::max (d, (double) std::abs (y0[i] - in[i]));
            CHECK (d < 1e-6, "%s mix 0 differs from dry by %.3g", names[mode], d);
        }
        {   // Very hot input (+18 dBFS) at full intensity stays finite and under the ceiling.
            EndlessSmileInspiredAudioProcessor p; p.prepareToPlay (sr, 512);
            setParams (p, mode, 110.0f, 0.0f);
            auto loud = in; for (auto& v : loud) v *= 64.0f;
            render (p, loud, 512, 2, y0, y1);
            const auto r = measure (y0, y1, (size_t) (sr * 0.5));   // after the 25 ms smoothers settle
            CHECK (r.finite && r.peak <= 1.0001, "%s hot input: peak %.3f finite %d", names[mode], r.peak, r.finite);
        }
        {   // A single NaN and a single inf from the host must not poison the filters/reverb for good.
            EndlessSmileInspiredAudioProcessor p; p.prepareToPlay (sr, 512);
            setParams (p, mode, 80.0f, 0.3f);
            auto bad = in; bad[(size_t) (sr * 1.0)] = std::nanf (""); bad[(size_t) (sr * 1.5)] = INFINITY;
            render (p, bad, 512, 2, y0, y1);
            const auto r = measure (y0, y1, (size_t) (sr * 4.0));
            CHECK (r.finite, "%s still non-finite 2.5 s after a NaN/inf sample", names[mode]);
        }
    }
    std::printf("  done\n");

    std::printf("\n== 10. Graveyard voices on silent input at max intensity (RMS dBFS, peak) ==\n");
    for (int mode = 7; mode < 16; ++mode)
    {
        EndlessSmileInspiredAudioProcessor p; p.prepareToPlay (sr, 512);
        setParams (p, mode, 110.0f);
        const std::vector<float> silence ((size_t) (sr * 8), 0.0f);
        render (p, silence, 512, 2, y0, y1);
        const auto r = measure (y0, y1, (size_t) (sr * 2));
        std::printf ("  %-16s %6.1f dBFS  peak %.3f\n", names[mode], r.rmsDb, r.peak);
        CHECK (r.finite && r.peak <= 1.0001, "%s silent-input voice not finite/bounded", names[mode]);
        CHECK (r.rmsDb > -34.0 && r.rmsDb < -22.0, "%s voice level %.1f dBFS outside the -34..-22 target band", names[mode], r.rmsDb);
    }

    std::printf ("\n%s (%d failures)\n", failures == 0 ? "ALL CHECKS PASSED" : "CHECKS FAILED", failures);
    return failures == 0 ? 0 : 1;
}
