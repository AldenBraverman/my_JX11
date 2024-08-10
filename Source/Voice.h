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
#include "Filter.h"

struct Voice // produce the next output sample for a given note
{
    int note;
    // int velocity;
    Oscillator osc1; // bring in and set up Oscillator class
    Oscillator osc2; // second oscillator for combining oscillators
    float saw; // "add new variable to the struct"
    Envelope env;
    Filter filter;
    float cutoff; // for filter cutoff keytracking

    float period; // "add new property to the Voice struct"
    
    float panLeft, panRight;
    
    float target;
    float glideRate;
    
    float filterMod;
    
    float filterQ;

    void reset() // also for initialization
    {
        note = 0;
        // velocity = 0;
        saw = 0.0f;

        osc1.reset();
        osc2.reset();
        env.reset();
        
        panLeft = 0.707f;
        panRight = 0.707f;
        filter.reset();
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
        
        output = filter.render(output);
        
        float envelope = env.nextValue();
        return output * envelope;
        
        // return envelope // for debugging the envelope
    }
    
    void release()
    {
        env.release(); // pass equest to the envelope
    }
    
    void updatePanning() // nice crossover for panning
    {
        float panning = std::clamp((note - 60.0f) / 24.0f, -1.0f, 1.0f); // TO-DO: CHANGE THIS TO A PLUGIN PARAMETER
        panLeft = std::sin(PI_OVER_4 * (1.0f - panning));
        panRight = std::sin(PI_OVER_4 * (1.0f + panning));
    }
    
    void updateLFO()
    {
        period += glideRate * (target - period);
        
        float modulatedCutoff = cutoff * std::exp(filterMod);
        modulatedCutoff = std::clamp(modulatedCutoff, 30.0f, 20000.0f);
        filter.updateCoefficients(modulatedCutoff, filterQ);
        // filter.updateCoefficients(cutoff, 0.707f); // 0.707 = sqrt(1/2), no resonance for the Q factor
    }
};
