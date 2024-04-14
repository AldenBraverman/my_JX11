/*
  ==============================================================================

    Oscillator.h
    Created: 15 Jan 2024 2:06:07pm
    Author:  Alden

  ==============================================================================
*/

#pragma once

#include <cmath>

const float TWO_PI = 6.2831853071795864f;
const float PI = 3.1415926535897932f;
const float PI_OVER_4 = 0.7853981633974483f;

class Oscillator
{
public:
    float amplitude = 1.0f;
    // float freq;
    // float sampleRate;
    // float phaseOffset;
    // int sampleIndex;
    // float inc;
    // float phase;

    // for sawtooth additive synthesis
    // float freq;
    // float sampleRate;
    // float phaseBL;
    
    // Band-Limited Impulse Train Algorithm
    float period = 0.0f;
    
    float modulation = 1.0f;

    void reset()
    {
        // sampleIndex = 0;
        inc = 0.0f; // determines how quickly the phase variable changes
        phase = 0.0f; // sine phase - also just phase reset lol -- for BLIT, phase keeps track of where you are in the sine wave (period (number of samples) between impulses?)
        // phaseBL = -0.5f;
        // phase = 1.5707963268f; // cosine phase - but actually plays a triangle because cosine has two samples every cycle
        
        // sin0 = amplitude * std::sin(phase * TWO_PI);
        // sin1 = amplitude * std::sin((phase - inc) * TWO_PI);
        // dsin = 2.0f * std::cos(inc * TWO_PI);
        sin0 = 0.0f;
        sin1 = 0.0f;
        dsin = 0.0f;
        dc = 0.0f;
    }

    
    float nextBandlimitedSample() // bandlimited = no frequencies over nyquist
    {
        /*
        phaseBL += inc; // works just like phase wrap but starts at a different offset of -0.5
        if (phaseBL >= 1.0f) {
            phaseBL -= 1.0f;
        }

        float output = 0.0f;
        float nyquist = sampleRate / 2.0f;
        float h = freq; // starts at the fundamental frequency
        float i = 1.0f; // harmonic index
        float m = 0.6366197724f; // 2 / pi
        while (h < nyquist) { // add up sine waves until nyquist limit
            output += m * std::sin(TWO_PI * phaseBL * i) / i; // get sine value for this harmonic
            h += freq; // incremented to get the frequency of the next harmonic
            i += 1.0f; // counts which harmonic we are at
            m = -m; // provides scaling factor 2 / pi
        }
        return output;
        */
    }
    

    float nextSample()
    {
        /*
        float output = amplitude * std::sin(TWO_PI * sampleIndex * freq / sampleRate + phaseOffset); // no longer need to call std::sin for every sample

        sampleIndex += 1;
        return output;
         */
        
        /*
        phase += inc; // wraps phase around sample window
        if (phase >= 1.0f) {
            phase -= 1.0f;
        }
        */
        
        // return amplitude * std::sin(TWO_PI * phase);
        // return amplitude * (2.0f * phase - 1.0f); // "native sawtooth" - very sharp looking waveform
        
        
        // clever sin wave implementation
        // float sinx = dsin * sin0 - sin1;
        // sin1 = sin0;
        // sin0 = sinx;
        // return sinx;

        // return amplitude * nextBandlimitedSample();
        
        // BLIT
        float output = 0.0f;
        /*
         update phase - in first half of sinc function, inc is positive and phase is incremented.
         in second half, inc is negative and phase is decremented.
         */
        phase += inc;
        
        if (phase <= PI_OVER_4) {
            /*
             at this point. the oscillator should start a new impulse. phase is measured in "samples * pi"
             if phase is less than pi/4, the end of the previous impulse is reached and we need to start a new one
             oscillator enters if statement once per period. and also immediately when a new note starts
             */
            /*
             in the next section, we need to find where the midpoint will be between the peak that was just finished and the next one
             midpoint will depend on the period, if the period is 100 samples, the next midpoint will be 50 samples into the future
             the period can change while the oscillator is active (user plays a new note or modulation effect is applied like vibrato)
             oscillator will ignore any changes to the period until it starts the next cycle
             phaseMax variable holds the position of the midpoint between the two impulse peaks, and again is measured in "samples * pi"
             the floor function is used to estimate the halfway point, this helps reduce aliasing
             this also eans inc isn't going to be exactly pi but a value close to it
             */
            float halfPeriod = (period / 2.0f) * modulation; // added vibrato modulation!
            phaseMax = std::floor(0.5f + halfPeriod) - 0.5f;
            dc = 0.5 * amplitude / phaseMax; // remove dc offset
            phaseMax *= PI;
            
            inc = phaseMax / halfPeriod;
            phase = -phase;
            
            /* 4
             Calculate sin(pi*x)/pi*x where phase is the variable that holds the current pi*x, this is multiplied by amplitude and becomes the ouput value for this sample
             sin(0)/0 gives us a division by zero, in this case, the sinc function should output the value 1
             floating point values are not always precise, we can't simply check if phase == 0.0
             */
            
            // Optimization
            sin0 = amplitude * std::sin(phase);
            sin1 = amplitude * std::sin(phase - inc);
            dsin = 2.0f * std::cos(inc);
            
            if (phase*phase > 1e-9) {
                // output = amplitude * std::sin(phase) / phase; // sinc function?
                output = sin0 / phase;
            } else {
                output = amplitude;
            }
        } else { // When we get here, the current sample is somewhere between the previous peak and the next, this is where the oscillator spends most of its time
            /*
             if the phase counter goes past the half-way point, set phase to the maximum and invert the increment inc, so that now the oscillator will begin outputting the sinc function backwards
             */
            if (phase > phaseMax) {
                phase = phaseMax + phaseMax - phase;
                inc = -inc;
            }
            // Calculate the current value for the sinc function, here you don't have to check for a possbile division by zero because phase will always be large enough
            // output = amplitude * std::sin(phase) / phase; // sinc function?
            // Optimization
            float sinp = dsin * sin0 - sin1;
            sin1 = sin0;
            sin0 = sinp;
            
            output = sinp / phase;
        }
        return output - dc; // output with dc offset removed (the low 0hz frequency is removed)
    }
    
    void squareWave(Oscillator& other, float newPeriod)
    {
        /* 1
         reset this oscillator
         */
        reset();
        
        /* 2
         figure out phase of the other oscillator and which direction it is going
         this is specific to the BLIT algorithm
         */
        if (other.inc > 0.0f) {
            phase = other.phaseMax + other.phaseMax - other.phase;
            inc = -other.inc;
        }
        else if (other.inc < 0.0f) {
            phase = other.phase;
            inc = other.inc;
        }
        else { // if other.inc equals zero, the other oscillator has not started and we dont know ehre its peaks will be, guess the value of inc as a value close to PI
            phase = -PI;
            inc = PI;
        }

        /* 3
         Shift phase by half a period so we are halfway between the peaks of the other oscillator
         phase measured in samples times PI
         newPeriod variable used here is the unmodulated period for the note to play
         */
        phase += PI * newPeriod / 2.0f;
        phaseMax = phase;
    }
private:
    float sin0; // private variable
    float sin1; // private variable
    float dsin; // private variable
    float phase;
    float phaseMax;
    float inc;
    float dc; // for removing DC offset
};
