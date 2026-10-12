#include "PluginEditor.h"
EndlessSmileInspiredAudioProcessorEditor::EndlessSmileInspiredAudioProcessorEditor(EndlessSmileInspiredAudioProcessor& p)
: AudioProcessorEditor(&p), processor(p), intensity("intensity", &p.state, "intensity"), preset("preset", &p.state, "preset"), mix("mix", &p.state, "mix")
{
    setSize(900,510); setResizable(true,true);
    intensity.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); intensity.setTextBoxStyle(juce::Slider::TextBoxBelow,false,90,24);
    intensity.setRange(0,110,.1); intensity.setNumDecimalPlacesToDisplay(1); addAndMakeVisible(intensity);
    preset.addItem("SOFT",1); preset.addItem("MEDIUM NO LIMIT",2); preset.addItem("SMACK IN THE FACE",3);
    preset.addItem("FIST IN THE AIR",4); preset.addItem("DADA DYNAMITE",5); preset.addItem("BROKEN TEETH",6); preset.addItem("EXTREME",7);
    addAndMakeVisible(preset);
    mix.setSliderStyle(juce::Slider::LinearHorizontal); mix.setTextBoxStyle(juce::Slider::TextBoxRight,false,55,22); mix.setRange(0,1,.01); addAndMakeVisible(mix);
    startTimerHz(30);
}
void EndlessSmileInspiredAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto b=getLocalBounds().toFloat();
    g.fillAll(juce::Colour(0xff565456));
    // paper/grain texture
    juce::Random r(91); for(int i=0;i<1300;++i){float x=r.nextFloat()*b.getWidth(), y=r.nextFloat()*b.getHeight(); g.setColour(juce::Colours::black.withAlpha(r.nextFloat()*.055f));g.fillEllipse(x,y,1+r.nextFloat()*2,1+r.nextFloat()*2);}
    g.setColour(juce::Colour(0xff29272a));g.fillRoundedRectangle(b.reduced(7),10);
    g.setColour(juce::Colour(0xff6a6768));g.fillRoundedRectangle(b.reduced(10),8);
    g.setColour(juce::Colour(0xffd0ceca).withAlpha(.18f));g.drawRoundedRectangle(b.reduced(10),8,2);
    g.setColour(juce::Colours::black);g.setFont(juce::Font(juce::FontOptions(17,juce::Font::bold)));g.drawText("INTENSITY",getWidth()/2-105,235,210,34,juce::Justification::centred);
    g.setColour(juce::Colour(0xffd5d0c9).withAlpha(.75f));g.setFont(juce::Font(juce::FontOptions(10,juce::Font::bold)));g.drawText("SKELLY AUDIO  •  BUILD-UP / TRANSITION EFFECT",20,getHeight()-27,getWidth()-40,16,juce::Justification::centredLeft);
    // Expression morphs with intensity: eyes + progressively wider smile.
    float x=getWidth()*.73f, y=getHeight()*.47f, rad=juce::jmin(getWidth()*.18f,getHeight()*.29f);
    float t=juce::jlimit(0.f,1.f,(float)intensity.getValue()/110.f);
    g.setColour(juce::Colour(0xffff9b2f));g.fillEllipse(x-rad*.95f,y-rad*.95f,rad*1.9f,rad*1.9f);
    g.setColour(juce::Colour(0xffd8f0df));g.fillEllipse(x-rad*.82f,y-rad*.80f,rad*1.64f,rad*1.58f);
    g.setColour(juce::Colour(0xfff24b91));g.setColour(juce::Colour(0xfff24b91));g.drawLine(x-rad*.82f,y-rad*.35f,x-rad*1.12f,y-rad*.98f,8);g.drawLine(x+rad*.82f,y-rad*.35f,x+rad*1.12f,y-rad*.98f,8);
    g.setColour(juce::Colour(0xffa42f59));g.fillEllipse(x-rad*.48f,y-rad*.28f,rad*.20f,rad*.31f);g.fillEllipse(x+rad*.28f,y-rad*.28f,rad*.20f,rad*.31f);
    juce::Path smile; float w=rad*(.55f+1.05f*t), h=rad*(.10f+.70f*t);
    smile.startNewSubPath(x-w,y+rad*.18f); smile.quadraticTo(x,y+rad*.18f+h*2,x+w,y+rad*.18f);
    g.setColour(juce::Colour(0xfff24b91));g.strokePath(smile,juce::PathStrokeType(11,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    juce::Path mouth; mouth.startNewSubPath(x-w,y+rad*.18f); mouth.quadraticTo(x,y+rad*.18f+h*2,x+w,y+rad*.18f); mouth.lineTo(x+w,y+rad*.22f);mouth.quadraticTo(x,y+rad*.22f+h*2,x-w,y+rad*.22f);mouth.closeSubPath();
    g.setColour(juce::Colour(0xff802747));g.fillPath(mouth);
    if(t>.22f){g.setColour(juce::Colour(0xfffff0d4));g.fillRoundedRectangle(x-w*.8f,y+rad*.22f,w*1.6f,juce::jmax(5.f,h*.34f),4);}
    g.setColour(juce::Colour(0xffdedbd6));g.drawArc(getWidth()*.17f,35,getWidth()*.25f,180,juce::MathConstants<float>::pi*1.12f,juce::MathConstants<float>::pi*1.88f,true,5);
}
void EndlessSmileInspiredAudioProcessorEditor::resized()
{
    auto b=getLocalBounds(); int dial=juce::jmin(220,b.getHeight()*0.43f);
    intensity.setBounds(b.getCentreX()-dial/2,30,dial,dial+28);
    preset.setBounds(35,b.getHeight()-120,b.getWidth()*.47f,38);
    mix.setBounds(35,b.getHeight()-70,b.getWidth()*.47f,30);
}
void EndlessSmileInspiredAudioProcessorEditor::timerCallback(){repaint();}
