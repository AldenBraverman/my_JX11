/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "RotaryKnob.h"
#include "LookAndFeel.h"

//==============================================================================
/**
*/


class My_JX11AudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    My_JX11AudioProcessorEditor (My_JX11AudioProcessor&);
    ~My_JX11AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    My_JX11AudioProcessor& audioProcessor;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using SliderAttachment = APVTS::SliderAttachment;
    using ButtonAttachment = APVTS::ButtonAttachment;
    
    RotaryKnob outputLevelKnob;
    SliderAttachment outputLevelAttachment {
        audioProcessor.apvts,
        ParameterID::outputLevel.getParamID(),
        outputLevelKnob.slider
    };
    
    RotaryKnob filterResoKnob;
    SliderAttachment filterResoAttachment {
        audioProcessor.apvts,
        ParameterID::filterReso.getParamID(),
        filterResoKnob.slider
    };
    
    juce::TextButton polyModeButton;
    ButtonAttachment polyModeAttachment {
        audioProcessor.apvts,
        ParameterID::polyMode.getParamID(),
        polyModeButton
    };
    
    LookAndFeel globalLNF;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (My_JX11AudioProcessorEditor)
};
