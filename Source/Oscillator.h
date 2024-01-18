/*
  ==============================================================================

    Oscillator.h
    Created: 15 Jan 2024 2:06:07pm
    Author:  Alden

  ==============================================================================
*/

#pragma once
const float TWO_PI = 6.2831853071795864f;

class Oscillator
{
public:
    float amplitude;
    // float freq;
    // float sampleRate;
    // float phaseOffset;
    // int sampleIndex;
    float inc;
    float phase;

    void reset()
    {
        // sampleIndex = 0;
        phase = 0; // sine phase - also just phase reset lol
        // phase = 1.5707963268f; // cosine phase - but actually plays a triangle because cosine has two samples every cycle
        
        // sin0 = amplitude * std::sin(phase * TWO_PI);
        // sin1 = amplitude * std::sin((phase - inc) * TWO_PI);
        // dsin = 2.0f * std::cos(inc * TWO_PI);
    }

    float nextSample()
    {
        /*
        float output = amplitude * std::sin(TWO_PI * sampleIndex * freq / sampleRate + phaseOffset); // no longer need to call std::sin for every sample

        sampleIndex += 1;
        return output;
         */
        
        
        phase += inc; // wraps phase around sample window
        if (phase >= 1.0f) {
            phase -= 1.0f;
        }
        
        // return amplitude * std::sin(TWO_PI * phase);
        return amplitude * (2.0f * phase - 1.0f); // "native sawtooth"
        
        
        // clever sin wave implementation
        // float sinx = dsin * sin0 - sin1;
        // sin1 = sin0;
        // sin0 = sinx;
        // return sinx;
    }
private:
    float sin0; // private variable
    float sin1; // private variable
    float dsin; // private variable
};
