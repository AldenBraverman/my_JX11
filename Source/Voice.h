/*
  ==============================================================================

    Voice.h
    Created: 12 Jan 2024 7:38:30pm
    Author:  Alden

  ==============================================================================
*/

#pragma once
#include "Oscillator.h"
#include "Envelope.h"

struct Voice // produce the next output sample for a given note
{
    int note;
    // int velocity;
    Oscillator osc1; // bring in and set up Oscillator class
    Oscillator osc2; // second oscillator for combining oscillators
    float saw; // "add new variable to the struct"
    Envelope env;

    void reset() // also for initialization
    {
        note = 0;
        // velocity = 0;
        saw = 0.0f;
        env.reset();
        osc1.reset();
        osc2.reset();
    }

    float render(float input)
    {
        // return osc.nextSample();
        // float sample = osc.nextSample();
        float sample1 = osc1.nextSample();
        float sample2 = osc2.nextSample();
        saw = saw * 0.997f + sample1 - sample2; // ramp up sawtooth
        // saw = saw * 0.997f - sample; // ramp down sawtooth
        float output = saw + input; // input is the noise signal
        float envelope = env.nextValue();

        return output * envelope;
        // return envelope // for debugging the envelope
    }
    
    void release()
    {
        env.release(); // pass equest to the envelope
    }
};
