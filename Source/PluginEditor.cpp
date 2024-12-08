/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
My_JX11AudioProcessorEditor::My_JX11AudioProcessorEditor (My_JX11AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
//    outputLevelKnob.setSliderStyle(
//                                   juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag
//                                   );
//    
//    outputLevelKnob.setTextBoxStyle(
//                                    juce::Slider::TextBoxBelow,
//                                    false,
//                                    100,
//                                    20
//                                    );
//    
//    addAndMakeVisible(outputLevelKnob);
//    
//    filterResoKnob.setSliderStyle(
//                                  juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag
//                                  );
//    
//    filterResoKnob.setTextBoxStyle(
//                                    juce::Slider::TextBoxBelow,
//                                    false,
//                                    100,
//                                    20
//                                    );
    juce::LookAndFeel::setDefaultLookAndFeel(&globalLNF);
    
    outputLevelKnob.label = "Level";
    addAndMakeVisible(outputLevelKnob);
    
    filterResoKnob.label = "Reso";
    addAndMakeVisible(filterResoKnob);
    
    polyModeButton.setButtonText("Poly");
    polyModeButton.setClickingTogglesState(true);
    addAndMakeVisible(polyModeButton);
    
    addAndMakeVisible(filterResoKnob);
    
    setSize (1200, 400);
}

My_JX11AudioProcessorEditor::~My_JX11AudioProcessorEditor()
{
}

//==============================================================================
void My_JX11AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

//    g.setColour (juce::Colours::white);
//    g.setFont (15.0f);
//    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void My_JX11AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    outputLevelKnob.setBounds(1100, 20, 100, 120);
    filterResoKnob.setBounds(120, 20, 100, 120);
    polyModeButton.setBounds(1100, 140, 80, 30);
    
//    juce::Rectangle r(20, 20, 100, 120);
////    juce::Rectangle r(20, 20, 200, 220);
//    
//    outputLevelKnob.setBounds(r);
//    
//    r = r.withX(r.getRight() + 20);
//    filterResoKnob.setBounds(r);
//    
//    polyModeButton.setSize(80, 30);
//    polyModeButton.setCentrePosition(r.withX(r.getRight()).getCentre());
}
