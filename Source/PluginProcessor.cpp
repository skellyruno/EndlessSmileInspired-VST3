#include "PluginProcessor.h"
#include "PluginEditor.h"

EndlessSmileInspiredAudioProcessor::EndlessSmileInspiredAudioProcessor()
: AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                  .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
  state(*this, nullptr, "PARAMETERS", createParameters())
{
    intensity = state.getRawParameterValue("intensity");
    preset = state.getRawParameterValue("preset");
    mix = state.getRawParameterValue("mix");
}
juce::AudioProcessorValueTreeState::ParameterLayout EndlessSmileInspiredAudioProcessor::createParameters()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;
    p.push_back(std::make_unique<AudioParameterFloat>("intensity","Intensity",NormalisableRange<float>(0.f,110.f,0.01f),0.f));
    p.push_back(std::make_unique<AudioParameterChoice>("preset","Preset",StringArray{"Soft","Medium No Limit","Smack in the Face","Fist in the Air","Dada Dynamite","Broken Teeth","Extreme"},0));
    p.push_back(std::make_unique<AudioParameterFloat>("mix","Mix",NormalisableRange<float>(0.f,1.f,0.001f),1.f));
    return {p.begin(),p.end()};
}
void EndlessSmileInspiredAudioProcessor::prepareToPlay(double sr,int)
{
    sampleRate=sr;
    juce::dsp::ProcessSpec spec{sr,(juce::uint32)juce::jmax(1,getBlockSize()),(juce::uint32)getTotalNumOutputChannels()};
    reverb.reset(); reverb.prepare(spec);
    juce::dsp::Reverb::Parameters rp; rp.roomSize=.8f; rp.damping=.35f; rp.wetLevel=.25f; rp.dryLevel=0.f; rp.width=1.f; reverb.setParameters(rp);
    for(auto& f: hp) { f.reset(); f.prepare(spec); f.setType(juce::dsp::StateVariableTPTFilterType::highpass); }
    for(auto& d: delay) { d.reset(); d.prepare(spec); d.setMaximumDelayInSamples((int)(sr*1.5)); }
    smoothIntensity.reset(sr,0.025); smoothMix.reset(sr,0.02);
    smoothIntensity.setCurrentAndTargetValue(*intensity); smoothMix.setCurrentAndTargetValue(*mix);
    noiseState=0x12345678u;
}
void EndlessSmileInspiredAudioProcessor::releaseResources(){}
bool EndlessSmileInspiredAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    return l.getMainInputChannelSet()==l.getMainOutputChannelSet() &&
      (l.getMainOutputChannelSet()==juce::AudioChannelSet::mono() || l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo());
}
void EndlessSmileInspiredAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto n=b.getNumSamples(); auto ch=b.getNumChannels();
    const float inten=juce::jlimit(0.f,1.1f,intensity->load()/100.f);
    const int mode=(int)preset->load();
    smoothIntensity.setTargetValue(inten); smoothMix.setTargetValue(mix->load());
    const float modeScale[7]={.42f,.55f,.72f,.62f,.82f,.95f,1.f};
    const float drive=1.f+inten*modeScale[mode]*3.5f;
    const float cutoff=juce::jmap(std::pow(inten,1.45f),20.f, mode<2?220.f:15000.f);
    for(int c=0;c<ch;++c) {
        hp[(size_t)c].setCutoffFrequency(cutoff);
        delay[(size_t)c].setDelay((float)juce::jlimit(1,(int)(sampleRate*1.45), (int)(sampleRate*(.12f+.22f*inten))));
    }
    juce::AudioBuffer<float> dry(b.getArrayOfWritePointers(),ch,n);
    // Reverb receives a copy so its wet tail is independent of the dry signal.
    juce::AudioBuffer<float> wet(ch,n); for(int c=0;c<ch;++c) wet.copyFrom(c,0,b,c,0,n);
    juce::dsp::AudioBlock<float> wetBlock(wet);
    juce::dsp::ProcessContextReplacing<float> rc(wetBlock); reverb.process(rc);
    for(int i=0;i<n;++i) {
        const float x=smoothIntensity.getNextValue();
        const float m=smoothMix.getNextValue();
        const float cut=juce::jmap(std::pow(x/1.1f,1.55f),20.f, mode<2?220.f:14500.f);
        for(int c=0;c<ch;++c) {
            float original=b.getSample(c,i);
            float filtered=hp[(size_t)c].processSample(c,original);
            float rv=wet.getSample(c,i);
            float delayed=delay[(size_t)c].popSample(c);
            delay[(size_t)c].pushSample(c,filtered);
            noiseState ^= noiseState << 13; noiseState ^= noiseState >> 17; noiseState ^= noiseState << 5;
            float noise=((float)(noiseState & 0xffff)/32767.5f-1.f);
            float noiseAmount=(mode>=2 ? .10f : .018f)*x*x;
            float shaped=std::tanh((filtered+noise*noiseAmount)*drive)/std::tanh(drive);
            float wetMix=juce::jlimit(0.f,.92f,x*modeScale[mode]*.72f);
            float delayMix=(mode==0?0.f:.05f)+x*(mode>=4?.24f:.12f);
            float riser=0.f;
            if(mode>=2 && x>.05f) {
                float phase=(float)(i+(int)(sampleRate*.1)) / (float)sampleRate;
                riser=noise*std::pow(x,2.f)*.035f*std::sin(phase*juce::MathConstants<float>::twoPi*(220.f+6000.f*x));
            }
            float processed=(1.f-wetMix-delayMix*.35f)*shaped + wetMix*rv + delayMix*delayed + riser;
            // Cutoff is continuously updated by intensity; blend to avoid a hard mute at 0.
            processed=juce::jmap(x/1.1f,original,processed);
            b.setSample(c,i,original*(1.f-m)+processed*m);
        }
    }
}
juce::AudioProcessorEditor* EndlessSmileInspiredAudioProcessor::createEditor(){return new EndlessSmileInspiredAudioProcessorEditor(*this);}
const juce::String EndlessSmileInspiredAudioProcessor::getName() const{return "Endless Smile Inspired";}
bool EndlessSmileInspiredAudioProcessor::acceptsMidi() const{return false;}
bool EndlessSmileInspiredAudioProcessor::producesMidi() const{return false;}
bool EndlessSmileInspiredAudioProcessor::isMidiEffect() const{return false;}
double EndlessSmileInspiredAudioProcessor::getTailLengthSeconds() const{return 4.0;}
int EndlessSmileInspiredAudioProcessor::getNumPrograms(){return 1;}
int EndlessSmileInspiredAudioProcessor::getCurrentProgram(){return 0;}
void EndlessSmileInspiredAudioProcessor::setCurrentProgram(int){}
const juce::String EndlessSmileInspiredAudioProcessor::getProgramName(int){return {};}
void EndlessSmileInspiredAudioProcessor::changeProgramName(int,const juce::String&){}
void EndlessSmileInspiredAudioProcessor::getStateInformation(juce::MemoryBlock& dest){auto xml=state.copyState().createXml();copyXmlToBinary(*xml,dest);}
void EndlessSmileInspiredAudioProcessor::setStateInformation(const void* data,int size){std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data,size));if(xml&&xml->hasTagName(state.state.getType()))state.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new EndlessSmileInspiredAudioProcessor();}
