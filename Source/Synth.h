/*
  ==============================================================================

    Synth.h
    Created: 12 Jan 2024 7:38:21pm
    Author:  Alden

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Voice.h" // import Voice class
#include "NoiseGenerator.h" // import NoiseGenerator class

class Synth // Synth has methods to reset its state, render current block of audio, and to manage MIDI messages
{
public:
    Synth();

    void allocateResources(double sampleRate, int samplesPerBlock); // analogous to prepareToPlay
    void deallocateResources(); // analogous to releaseResources
    void reset();
    void render(float** outputBuffers, int sampleCount);
    void midiMessage(uint8_t data0, uint8_t data1, uint8_t data2);

private:
    float sampleRate;
    Voice voice; // Set up Voice class
    void noteOn(int note, int velocity);
    void noteOff(int note);
    NoiseGenerator noiseGen; // Set up NoiseGenerator class
};