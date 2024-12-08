/*
  ==============================================================================

    RotaryKnob.cpp
    Created: 25 Nov 2024 10:38:41pm
    Author:  Alden Braverman

  ==============================================================================
*/

#include <JuceHeader.h>
#include "RotaryKnob.h"

//==============================================================================
static constexpr int labelHeight = 15;
static constexpr int textBoxHeight = 20;

RotaryKnob::RotaryKnob()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, textBoxHeight);
    addAndMakeVisible(slider);
    
    setBounds(0, 0, 100, 120);
}

RotaryKnob::~RotaryKnob()
{
}

void RotaryKnob::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    
    g.setFont(15.0f);
    g.setColour(juce::Colours::white);
    
    auto bounds = getLocalBounds();
    g.drawText(label, juce::Rectangle<int>{ 0, 0, bounds.getWidth(), labelHeight }, juce::Justification::centred);

//    g.setColour (juce::Colours::grey);
//    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
//
//    g.setColour (juce::Colours::white);
//    g.setFont (juce::FontOptions (14.0f));
//    g.drawText ("RotaryKnob", getLocalBounds(),
//                juce::Justification::centred, true);   // draw some placeholder text
    
    // debug bounds!
//    g.setColour(juce::Colours::red);
//    g.drawRect(getLocalBounds(),1);
    
    g.setColour(juce::Colours::yellow);
    g.drawRect(0, labelHeight, bounds.getWidth(), bounds.getHeight() - labelHeight - textBoxHeight, 1);
    
    g.setColour(juce::Colours::green);
    g.drawRect(0, 0, bounds.getWidth(), labelHeight, 1);
}

void RotaryKnob::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    auto bounds = getLocalBounds();
    slider.setBounds(0, labelHeight, bounds.getWidth(), bounds.getHeight() - labelHeight);
}
