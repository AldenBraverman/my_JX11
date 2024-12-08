/*
  ==============================================================================

    LookAndFeel.h
    Created: 7 Dec 2024 5:47:08pm
    Author:  Alden Braverman

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class LookAndFeel : public juce::LookAndFeel_V4 {
public:
    LookAndFeel();
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LookAndFeel)
};
