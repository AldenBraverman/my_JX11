/*
  ==============================================================================

    Voice.h
    Created: 12 Jan 2024 7:38:30pm
    Author:  Alden

  ==============================================================================
*/

#pragma once
#include "Oscillator.h"

struct Voice // produce the next output sample for a given note
{
    int note;
    // int velocity;
    Oscillator osc; // bring in and set up Oscillator class
    float saw; // "add new variable to the struct"

    void reset() // also for initialization
    {
        note = 0;
        // velocity = 0;
        saw = 0.0f;
    }

    float render()
    {
        // return osc.nextSample();
        float sample = osc.nextSample();
        saw = saw * 0.997f + sample; // ramp up sawtooth
        // saw = saw * 0.997f - sample; // ramp down sawtooth
        return saw;
    }
};
