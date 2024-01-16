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

    void reset() // also for initialization
    {
        note = 0;
        // velocity = 0;
    }

    float render()
    {
        return osc.nextSample();
    }
};