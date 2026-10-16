#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    // Presets 0-6 are the original seven; 7-11 are the graveyard presets.
    constexpr int presetCount = 12;

    // Length of one rise in the Hard presets (Smack You in the Face, Broken Teeth).
    // Placeholder pending a decision: a fixed time in seconds, or tempo-synced.
    constexpr float hardRiserLoopSeconds = 8.0f;

    // Transparent below 0.9, then rolls off smoothly so the effect never exceeds 1.0.
    inline float softCeiling (float x) noexcept
    {
        const float a = std::abs (x);
        if (a <= 0.9f)
            return x;
        return std::copysign (0.9f + 0.1f * std::tanh ((a - 0.9f) / 0.1f), x);
    }
}

EndlessSmileInspiredAudioProcessor::EndlessSmileInspiredAudioProcessor()
    : AudioProcessor (BusesProperties().withInput  ("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      state (*this, nullptr, "PARAMETERS", createParameters())
{
    intensityParameter = state.getRawParameterValue ("intensity");
    presetParameter = state.getRawParameterValue ("preset");
    mixParameter = state.getRawParameterValue ("mix");
}

juce::AudioProcessorValueTreeState::ParameterLayout EndlessSmileInspiredAudioProcessor::createParameters()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> parameters;
    parameters.push_back (std::make_unique<AudioParameterFloat> (
        ParameterID { "intensity", 1 }, "Intensity", NormalisableRange<float> (0.0f, 110.0f, 0.01f), 0.0f));
    parameters.push_back (std::make_unique<AudioParameterChoice> (
        ParameterID { "preset", 1 }, "Preset",
        StringArray { "Fist in the Air", "Helium Jumping", "No Limit", "Face Melting Joy",
                     "Smack You in the Face", "Broken Teeth", "Dada Dynamite",
                     "Midnight Howl", "Banshee Scream", "Ghost Whisper",
                     "Rattling Bones", "Crypt Drone" }, 0));
    parameters.push_back (std::make_unique<AudioParameterFloat> (
        ParameterID { "mix", 1 }, "Mix", NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));
    return { parameters.begin(), parameters.end() };
}

void EndlessSmileInspiredAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    currentSampleRate = sampleRate;

    reverb.setSampleRate (sampleRate);

    juce::Reverb::Parameters reverbParameters;
    reverbParameters.roomSize = 0.78f;
    reverbParameters.damping = 0.28f;
    reverbParameters.wetLevel = 0.0f; // updated smoothly per block from intensity
    reverbParameters.dryLevel = 0.0f; // wet-only: the dry path is summed explicitly at unity gain
    reverbParameters.width = 1.0f;
    reverb.setParameters (reverbParameters);
    reverb.reset();

    juce::dsp::ProcessSpec monoSpec { sampleRate, (juce::uint32) juce::jmax (1, getBlockSize()), 1 };
    for (auto& filter : highPassFilters)
    {
        filter.reset();
        filter.prepare (monoSpec);
        filter.setType (juce::dsp::StateVariableTPTFilterType::highpass);
        filter.setCutoffFrequency (20.0f);
        filter.setResonance (0.707f);
    }

    for (auto& delay : delayLines)
    {
        delay.setMaximumDelayInSamples (juce::jmax (2, (int) std::ceil (sampleRate * 1.25)));
        delay.prepare (monoSpec);
        delay.reset();
        delay.setDelay ((float) (sampleRate * 0.22));
    }

    for (auto& filter : scaryBandFilters)
    {
        filter.reset();
        filter.prepare (monoSpec);
        filter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
        filter.setCutoffFrequency (800.0f);
        filter.setResonance (5.0f);
    }
    lfoPhaseA = lfoPhaseB = lfoPhaseC = lfoPhaseD = 0.0f;
    vibratoPhase = scarePhase = scarePhase2 = 0.0f;
    clickEnv[0] = clickEnv[1] = 0.0f;
    feedbackDamp[0] = feedbackDamp[1] = 0.0f;

    smoothedIntensity.reset (sampleRate, 0.025);
    smoothedMix.reset (sampleRate, 0.02);
    // Slow glide on delay time so moving the knob never clicks or zippers the echo.
    smoothedDelaySeconds.reset (sampleRate, 0.30);
    smoothedIntensity.setCurrentAndTargetValue (intensityParameter->load());
    smoothedMix.setCurrentAndTargetValue (mixParameter->load());
    smoothedDelaySeconds.setCurrentAndTargetValue (0.12f + 0.20f * (intensityParameter->load() / 110.0f));
    noiseState = 0x12345678u;
    noiseStateRight = 0x9e3779b9u;
    riserPhase = 0.0f;
    riserRamp = 0.0f;
}

void EndlessSmileInspiredAudioProcessor::releaseResources() {}

bool EndlessSmileInspiredAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    return layouts.getMainInputChannelSet() == output
        && (output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo());
}

void EndlessSmileInspiredAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);
    juce::ScopedNoDenormals noDenormals;

    const int channels = juce::jmin (2, buffer.getNumChannels());
    const int samples = buffer.getNumSamples();
    if (channels == 0 || samples == 0)
        return;

    for (int channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear (channel, 0, samples);

    const int mode = juce::jlimit (0, presetCount - 1, juce::roundToInt (presetParameter->load()));
    const bool graveyard = (mode >= 7);
    const bool hasDelay = (mode == 1 || mode == 3 || mode == 5 || mode == 6 || mode == 8 || mode == 9);
    const bool hasNoiseAndRiser = (mode >= 4 && mode <= 6);
    const bool softHighPass = (mode <= 1);
    const float feedbackBase = (mode == 9) ? 0.42f : 0.0f;   // Ghost Whisper's echo trails off slowly

    const float intensityTarget = intensityParameter->load();
    smoothedIntensity.setTargetValue (intensityTarget);
    smoothedMix.setTargetValue (mixParameter->load());
    smoothedDelaySeconds.setTargetValue (mode == 9 ? 0.36f
                                         : (mode == 8 ? 0.27f : 0.12f + 0.20f * (intensityTarget / 110.0f)));

    // Reverb is present in every mode. The effect amount follows the smoothed
    // intensity so reverb settings do not jump between blocks.
    const float blockIntensity = juce::jlimit (0.0f, 1.0f, smoothedIntensity.getCurrentValue() / 110.0f);
    auto reverbParameters = reverb.getParameters();
    reverbParameters.roomSize = graveyard ? 0.84f + 0.14f * blockIntensity : 0.72f + 0.22f * blockIntensity;
    reverbParameters.damping = graveyard ? 0.45f : 0.30f;
    reverbParameters.wetLevel = graveyard ? 0.14f + 0.40f * blockIntensity : 0.08f + 0.42f * blockIntensity;
    reverbParameters.dryLevel = 0.0f; // wet-only; the dry signal is added back at unity below
    reverbParameters.width = 1.0f;
    reverb.setParameters (reverbParameters);

    const float sr = (float) currentSampleRate;
    const float twoPi = juce::MathConstants<float>::twoPi;
    const float lfoAStep = twoPi * 0.13f / sr;
    const float lfoBStep = twoPi * 0.37f / sr;
    const float lfoCStep = twoPi * 0.22f / sr;
    const float lfoDStep = twoPi * 0.33f / sr;
    const float vibratoStep = twoPi * 6.1f / sr;
    const float clickDecay = std::exp (-1.0f / (0.012f * sr));
    const float maxBandCentre = sr * 0.45f;
    const auto wrapPhase = [twoPi] (float& phase) noexcept { if (phase >= twoPi) phase -= twoPi; };

    if (graveyard)
    {
        const float bandQ = mode == 7 ? 10.0f : (mode == 8 ? 3.5f : (mode == 9 ? 5.0f : (mode == 10 ? 2.5f : 4.5f)));
        for (auto& filter : scaryBandFilters)
        {
            filter.setResonance (bandQ);
            if (mode == 10)
                filter.setCutoffFrequency (3000.0f);
        }
    }

    const auto nextNoise = [] (uint32_t& st) noexcept
    {
        st ^= st << 13;
        st ^= st >> 17;
        st ^= st << 5;
        return ((float) (st & 0xffffu) / 32767.5f) - 1.0f;
    };

    for (int i = 0; i < samples; ++i)
    {
        const float intensity = juce::jlimit (0.0f, 110.0f, smoothedIntensity.getNextValue()) / 110.0f;
        const float mix = juce::jlimit (0.0f, 1.0f, smoothedMix.getNextValue());
        const float maxCutoff = softHighPass ? 200.0f : (graveyard ? 600.0f : 15000.0f);
        const float cutoff = juce::jmap (std::pow (intensity, 1.35f), 20.0f, maxCutoff);
        for (auto& filter : highPassFilters)
            filter.setCutoffFrequency (cutoff);

        const float delaySamples = juce::jlimit (1.0f, (float) (currentSampleRate * 1.24),
                                                 smoothedDelaySeconds.getNextValue() * (float) currentSampleRate);
        for (auto& delay : delayLines)
            delay.setDelay (delaySamples);

        const float noiseLeft = nextNoise (noiseState);
        const float noiseRight = (channels == 2) ? nextNoise (noiseStateRight) : noiseLeft;

        // ---- Graveyard voices: synthesised layers that grow with intensity ----
        float layer[2] { 0.0f, 0.0f };
        if (graveyard)
        {
            const float sweepA = 0.5f + 0.5f * std::sin (lfoPhaseA);
            switch (mode)
            {
                case 7: // Midnight Howl: wind through a resonant band that drifts up and down
                {
                    const float gust = 0.55f + 0.45f * std::sin (lfoPhaseB);
                    const float centre = juce::jlimit (60.0f, maxBandCentre, 330.0f + (420.0f + 900.0f * intensity) * sweepA);
                    const float level = 0.22f * gust * std::pow (intensity, 1.2f);
                    for (int channel = 0; channel < channels; ++channel)
                    {
                        scaryBandFilters[(size_t) channel].setCutoffFrequency (centre);
                        layer[channel] = scaryBandFilters[(size_t) channel].processSample (0, channel == 0 ? noiseLeft : noiseRight) * level;
                    }
                    break;
                }
                case 8: // Banshee Scream: rising, vibrato-laden voice through a vowel-like formant
                {
                    const float pitch = 520.0f + 1700.0f * std::pow (intensity, 1.4f)
                                      + 38.0f * std::sin (vibratoPhase) * (0.4f + intensity);
                    scarePhase += twoPi * pitch / sr;
                    wrapPhase (scarePhase);
                    const float osc = std::sin (scarePhase) + 0.55f * std::sin (2.0f * scarePhase)
                                    + 0.30f * std::sin (3.0f * scarePhase);
                    const float formant = juce::jlimit (60.0f, maxBandCentre, 1500.0f + 900.0f * intensity);
                    const float level = 0.42f * std::pow (intensity, 1.1f);
                    for (int channel = 0; channel < channels; ++channel)
                    {
                        scaryBandFilters[(size_t) channel].setCutoffFrequency (formant);
                        const float voiced = scaryBandFilters[(size_t) channel].processSample (0, osc);
                        const float breath = (channel == 0 ? noiseLeft : noiseRight) * 0.075f;
                        layer[channel] = (std::tanh (voiced * 1.6f) + breath) * level;
                    }
                    break;
                }
                case 9: // Ghost Whisper: breathy, fluttering band of air (the echo does the haunting)
                {
                    const float centre = juce::jlimit (60.0f, maxBandCentre,
                                                       1800.0f + 2600.0f * (0.5f + 0.5f * std::sin (lfoPhaseC)));
                    const float trem = 0.5f + 0.5f * std::sin (lfoPhaseD);
                    const float level = 0.22f * trem * trem * intensity;
                    for (int channel = 0; channel < channels; ++channel)
                    {
                        scaryBandFilters[(size_t) channel].setCutoffFrequency (centre);
                        layer[channel] = scaryBandFilters[(size_t) channel].processSample (0, channel == 0 ? noiseLeft : noiseRight) * level;
                    }
                    break;
                }
                case 10: // Rattling Bones: random dry clicks that get denser as intensity rises
                {
                    const float triggerChance = (3.0f + 55.0f * intensity * intensity) / sr;
                    const float level = 0.45f * std::pow (intensity, 0.8f);
                    for (int channel = 0; channel < channels; ++channel)
                    {
                        const float own = channel == 0 ? noiseLeft : noiseRight;
                        const float other = channel == 0 ? noiseRight : noiseLeft;
                        if (0.5f * (own + 1.0f) < triggerChance)
                            clickEnv[channel] = 0.5f + 0.5f * std::abs (other);
                        const float click = other * clickEnv[channel];
                        clickEnv[channel] *= clickDecay;
                        layer[channel] = scaryBandFilters[(size_t) channel].processSample (0, click) * level;
                    }
                    break;
                }
                default: // 11, Crypt Drone: two detuned low buzzes through a slowly moving vowel
                {
                    const float base = 46.0f * (1.0f + 0.012f * std::sin (lfoPhaseA));
                    scarePhase += twoPi * base / sr;
                    scarePhase2 += twoPi * base * 1.0113f / sr;
                    wrapPhase (scarePhase);
                    wrapPhase (scarePhase2);
                    const auto stack = [] (float ph) noexcept
                    {
                        return std::sin (ph) + 0.5f * std::sin (2.0f * ph)
                             + 0.33f * std::sin (3.0f * ph) + 0.25f * std::sin (4.0f * ph);
                    };
                    const float drone = 0.5f * (stack (scarePhase) + stack (scarePhase2));
                    const float centre = juce::jlimit (60.0f, maxBandCentre, 220.0f + 380.0f * sweepA);
                    const float level = 0.42f * std::pow (intensity, 0.9f);
                    for (int channel = 0; channel < channels; ++channel)
                    {
                        scaryBandFilters[(size_t) channel].setCutoffFrequency (centre);
                        layer[channel] = scaryBandFilters[(size_t) channel].processSample (0, drone) * level;
                    }
                    break;
                }
            }

            lfoPhaseA += lfoAStep;  wrapPhase (lfoPhaseA);
            lfoPhaseB += lfoBStep;  wrapPhase (lfoPhaseB);
            lfoPhaseC += lfoCStep;  wrapPhase (lfoPhaseC);
            lfoPhaseD += lfoDStep;  wrapPhase (lfoPhaseD);
            vibratoPhase += vibratoStep;  wrapPhase (vibratoPhase);
        }

        float dry[2] { 0.0f, 0.0f };
        float highPassed[2] { 0.0f, 0.0f };
        float filtered[2] { 0.0f, 0.0f };
        float delayed[2] { 0.0f, 0.0f };
        for (int channel = 0; channel < channels; ++channel)
        {
            dry[channel] = buffer.getSample (channel, i);
            highPassed[channel] = highPassFilters[(size_t) channel].processSample (0, dry[channel]);
        }

        // JUCE Reverb API expects channel pointers plus a sample count.
        // This loop processes one frame at a time, so the sample count is 1.
        // The reverb runs wet-only; the high-passed dry signal is added back at unity gain.
        // Graveyard voices are also fed into the reverb so they get a long haunted tail.
        float wet[2] { highPassed[0] + layer[0] * 0.5f, highPassed[1] + layer[1] * 0.5f };
        if (channels == 2)
            reverb.processStereo (&wet[0], &wet[1], 1);
        else
            reverb.processMono (&wet[0], 1);

        for (int channel = 0; channel < channels; ++channel)
            filtered[channel] = highPassed[channel] + wet[channel];

        const float feedback = feedbackBase * (0.5f + 0.5f * intensity);
        for (int channel = 0; channel < channels; ++channel)
        {
            if (hasDelay)
            {
                const float echo = delayLines[(size_t) channel].popSample (0);
                delayed[channel] = echo;
                feedbackDamp[channel] += 0.30f * (echo - feedbackDamp[channel]);
                delayLines[(size_t) channel].pushSample (0, filtered[channel] + feedbackDamp[channel] * feedback);
            }
            else
            {
                // Keep the delay buffers advancing so switching presets doesn't release stale repeats.
                delayLines[(size_t) channel].popSample (0);
                delayLines[(size_t) channel].pushSample (0, 0.0f);
                feedbackDamp[channel] = 0.0f;
            }
        }

        const float noiseAmount = hasNoiseAndRiser ? (0.025f + 0.16f * intensity) * intensity : 0.0f;

        float riserFrequency = 80.0f;
        float riserLevel = 0.0f;
        if (mode == 6)
        {
            // Extreme: noise and riser are driven directly by the intensity knob.
            riserFrequency = 80.0f + 9000.0f * intensity * intensity;
            riserLevel = intensity * intensity * 0.12f;
        }
        else if (hasNoiseAndRiser)
        {
            // Hard: a continual riser that keeps climbing and restarts, with short
            // fades at each end of the loop so the restart never clicks.
            riserRamp += 1.0f / (hardRiserLoopSeconds * (float) currentSampleRate);
            if (riserRamp >= 1.0f)
                riserRamp -= 1.0f;
            const float fade = juce::jmin (1.0f, riserRamp / 0.03f, (1.0f - riserRamp) / 0.05f);
            riserFrequency = 80.0f + 9000.0f * riserRamp * riserRamp;
            riserLevel = (0.30f + 0.70f * intensity) * 0.10f * fade;
        }
        const float riser = hasNoiseAndRiser ? std::sin (riserPhase) * riserLevel : 0.0f;
        riserPhase += juce::MathConstants<float>::twoPi * riserFrequency / (float) currentSampleRate;
        if (riserPhase >= juce::MathConstants<float>::twoPi)
            riserPhase -= juce::MathConstants<float>::twoPi;

        const float drive = 1.0f + intensity * (graveyard ? 1.2f : (mode < 2 ? 1.8f : (mode < 4 ? 2.8f : 4.2f)));
        // Level compensation: a hard-driven saturator is far louder than the dry signal
        // (measured +9 to +21 dB at full intensity), so scale it back as drive rises.
        const float makeup = std::pow (drive, -0.55f);
        const float delayAmount = hasDelay ? intensity * (mode == 6 ? 0.32f : (graveyard ? 0.30f : 0.24f)) : 0.0f;
        const float reverbAmount = graveyard ? 0.22f + intensity * 0.40f : 0.10f + intensity * 0.32f;

        for (int channel = 0; channel < channels; ++channel)
        {
            const float noiseSample = (channel == 0) ? noiseLeft : noiseRight;
            // Noise and riser are added after the saturator at fixed, modest levels
            // instead of being amplified by the drive stage.
            const float saturated = std::tanh (filtered[channel] * drive) * makeup;
            float effected = saturated * (1.0f - reverbAmount * 0.35f - delayAmount * 0.35f)
                           + filtered[channel] * reverbAmount * 0.35f
                           + delayed[channel] * delayAmount
                           + layer[channel] * 0.7f
                           + noiseSample * noiseAmount * 0.45f
                           + riser * 0.55f;
            effected = softCeiling (effected);
            effected = juce::jmap (intensity, dry[channel], effected);
            buffer.setSample (channel, i, dry[channel] * (1.0f - mix) + effected * mix);
        }
    }
}

juce::AudioProcessorEditor* EndlessSmileInspiredAudioProcessor::createEditor()
{
    return new EndlessSmileInspiredAudioProcessorEditor (*this);
}
const juce::String EndlessSmileInspiredAudioProcessor::getName() const { return "Skeleton Graveyard"; }
bool EndlessSmileInspiredAudioProcessor::acceptsMidi() const { return false; }
bool EndlessSmileInspiredAudioProcessor::producesMidi() const { return false; }
bool EndlessSmileInspiredAudioProcessor::isMidiEffect() const { return false; }
double EndlessSmileInspiredAudioProcessor::getTailLengthSeconds() const { return 4.0; }
int EndlessSmileInspiredAudioProcessor::getNumPrograms() { return 1; }
int EndlessSmileInspiredAudioProcessor::getCurrentProgram() { return 0; }
void EndlessSmileInspiredAudioProcessor::setCurrentProgram (int) {}
const juce::String EndlessSmileInspiredAudioProcessor::getProgramName (int) { return {}; }
void EndlessSmileInspiredAudioProcessor::changeProgramName (int, const juce::String&) {}
void EndlessSmileInspiredAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = state.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}
void EndlessSmileInspiredAudioProcessor::setStateInformation (const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, size));
    if (xml && xml->hasTagName (state.state.getType()))
        state.replaceState (juce::ValueTree::fromXml (*xml));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EndlessSmileInspiredAudioProcessor();
}
