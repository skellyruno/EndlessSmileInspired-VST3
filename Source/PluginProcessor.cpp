#include "PluginProcessor.h"
#include "PluginEditor.h"

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
                     "Smack You in the Face", "Broken Teeth", "Dada Dynamite" }, 0));
    parameters.push_back (std::make_unique<AudioParameterFloat> (
        ParameterID { "mix", 1 }, "Mix", NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));
    return { parameters.begin(), parameters.end() };
}

void EndlessSmileInspiredAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    currentSampleRate = sampleRate;

    juce::Reverb::Parameters reverbParameters;
    reverbParameters.roomSize = 0.78f;
    reverbParameters.damping = 0.28f;
    reverbParameters.wetLevel = 0.0f; // updated smoothly per block from intensity
    reverbParameters.dryLevel = 1.0f;
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

    smoothedIntensity.reset (sampleRate, 0.025);
    smoothedMix.reset (sampleRate, 0.02);
    smoothedIntensity.setCurrentAndTargetValue (intensityParameter->load());
    smoothedMix.setCurrentAndTargetValue (mixParameter->load());
    noiseState = 0x12345678u;
    riserPhase = 0.0f;
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

    const int mode = juce::jlimit (0, 6, juce::roundToInt (presetParameter->load()));
    const bool hasDelay = (mode == 1 || mode == 3 || mode == 5 || mode == 6);
    const bool hasNoiseAndRiser = (mode >= 4);
    const bool softHighPass = (mode <= 1);

    smoothedIntensity.setTargetValue (intensityParameter->load());
    smoothedMix.setTargetValue (mixParameter->load());

    // Reverb is present in all seven modes. The effect amount follows intensity.
    auto reverbParameters = reverb.getParameters();
    reverbParameters.roomSize = 0.72f + 0.22f * (intensityParameter->load() / 110.0f);
    reverbParameters.damping = 0.30f;
    reverbParameters.wetLevel = 0.08f + 0.42f * (intensityParameter->load() / 110.0f);
    reverbParameters.dryLevel = 1.0f;
    reverbParameters.width = 1.0f;
    reverb.setParameters (reverbParameters);

    const float delaySeconds = 0.12f + 0.20f * (intensityParameter->load() / 110.0f);
    for (auto& delay : delayLines)
        delay.setDelay (juce::jlimit (1.0f, (float) (currentSampleRate * 1.24), delaySeconds * (float) currentSampleRate));

    for (int i = 0; i < samples; ++i)
    {
        const float intensity = juce::jlimit (0.0f, 110.0f, smoothedIntensity.getNextValue()) / 110.0f;
        const float mix = juce::jlimit (0.0f, 1.0f, smoothedMix.getNextValue());
        const float maxCutoff = softHighPass ? 200.0f : 15000.0f;
        const float cutoff = juce::jmap (std::pow (intensity, 1.35f), 20.0f, maxCutoff);
        for (auto& filter : highPassFilters)
            filter.setCutoffFrequency (cutoff);

        float dry[2] { 0.0f, 0.0f };
        float filtered[2] { 0.0f, 0.0f };
        float delayed[2] { 0.0f, 0.0f };
        for (int channel = 0; channel < channels; ++channel)
        {
            dry[channel] = buffer.getSample (channel, i);
            filtered[channel] = highPassFilters[(size_t) channel].processSample (0, dry[channel]);
        }

        // Reverb processes a mono or stereo frame without allocating in the audio callback.
        if (channels == 2)
            reverb.processStereo (filtered[0], filtered[1]);
        else
            reverb.processMono (filtered[0]);

        for (int channel = 0; channel < channels; ++channel)
        {
            if (hasDelay)
            {
                delayed[channel] = delayLines[(size_t) channel].popSample (0);
                delayLines[(size_t) channel].pushSample (0, filtered[channel]);
            }
            else
            {
                // Keep the delay buffers advancing so switching presets doesn't release stale repeats.
                delayLines[(size_t) channel].popSample (0);
                delayLines[(size_t) channel].pushSample (0, 0.0f);
            }
        }

        noiseState ^= noiseState << 13;
        noiseState ^= noiseState >> 17;
        noiseState ^= noiseState << 5;
        const float noise = ((float) (noiseState & 0xffffu) / 32767.5f) - 1.0f;
        const float noiseAmount = hasNoiseAndRiser ? (0.025f + 0.16f * intensity) * intensity : 0.0f;
        const float riserFrequency = 80.0f + 9000.0f * intensity * intensity;
        const float riser = hasNoiseAndRiser ? std::sin (riserPhase) * intensity * intensity * 0.12f : 0.0f;
        riserPhase += juce::MathConstants<float>::twoPi * riserFrequency / (float) currentSampleRate;
        if (riserPhase >= juce::MathConstants<float>::twoPi)
            riserPhase -= juce::MathConstants<float>::twoPi;

        const float drive = 1.0f + intensity * (mode < 2 ? 1.8f : (mode < 4 ? 2.8f : 4.2f));
        const float delayAmount = hasDelay ? intensity * (mode == 6 ? 0.32f : 0.24f) : 0.0f;
        const float reverbAmount = 0.10f + intensity * 0.32f;

        for (int channel = 0; channel < channels; ++channel)
        {
            const float modulated = filtered[channel] + noise * noiseAmount + riser;
            const float saturated = std::tanh (modulated * drive) / std::tanh (drive);
            float effected = saturated * (1.0f - reverbAmount * 0.35f - delayAmount * 0.35f)
                           + filtered[channel] * reverbAmount * 0.35f
                           + delayed[channel] * delayAmount;
            effected = juce::jmap (intensity, dry[channel], effected);
            buffer.setSample (channel, i, dry[channel] * (1.0f - mix) + effected * mix);
        }
    }
}

juce::AudioProcessorEditor* EndlessSmileInspiredAudioProcessor::createEditor()
{
    return new EndlessSmileInspiredAudioProcessorEditor (*this);
}
const juce::String EndlessSmileInspiredAudioProcessor::getName() const { return "Endless Smile Inspired"; }
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
