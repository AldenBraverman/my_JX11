/*
  ==============================================================================

    Synth.cpp
    Created: 12 Jan 2024 7:38:21pm
    Author:  Alden

  ==============================================================================
*/

#include "Synth.h"
#include "Utils.h"

Synth::Synth()
{
    sampleRate = 44100.0f;
}

void Synth::allocateResources(double sampleRate_, int /* samplesPerBlock*/)
{
    sampleRate = static_cast<float>(sampleRate_);
}

void Synth::deallocateResources()
{
    // do nothing
}

void Synth::reset()
{
    voice.reset();
    noiseGen.reset();
    pitchBend = 1.0f;
}

void Synth::render(float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];
    
    voice.osc1.period = voice.period * pitchBend; // FORGOT THESE LINES, NO SOUND WITHOUT THEM
    voice.osc2.period = voice.osc1.period * detune;

    // Loop through smaples in buffer
    // sampleCount is the number of samples we need to render, if there were midi messages, sampleCount will be less than the total number of samples in the block
    for (int sample = 0; sample < sampleCount; ++sample) {
        
        // Get next output from noise gen
        float noise = noiseGen.nextValue() * noiseMix; // added noiseMix control parameter

        // check if voice.note is not 0 (a key is pressed - synth recieved noteOn but not noteOff)
        //float output = 0.0f;
        // separate output variables for the left and right channels
        float outputLeft = 0.0f;
        float outputRight = 0.0f;
        
        
        if (voice.env.isActive()) { // originally was voice.note > 0
            // Noise value multiplied by velocity
            // output = noise * (voice.velocity / 127.0f) * 0.5f; // Multiplying the output by 0.5 = 6 dB reduction in gain
            // output = voice.render(noise);// +noise; // instead of using output of noise gen, now we ask VOice object to produce next value for sin wave - update, added noise mix parameter - update, envelope affects noise now
            // sample renders in stereo
            float output = voice.render(noise);
            outputLeft += output * voice.panLeft;
            outputRight += output * voice.panRight;
        }
        // Write output value into audio buffers with mono/stereo logic
        // outputBufferLeft[sample] = output;
        // write sample values for left and right channels to their respective audio buffers
        if (outputBufferRight != nullptr) {
            outputBufferLeft[sample] = outputLeft;
            outputBufferRight[sample] = outputRight;
        } else {
            outputBufferLeft[sample] = (outputLeft + outputRight) * 0.5;
        }
    }
    
    if (!voice.env.isActive()) {
        voice.env.reset();
    }
    
    protectYourEars(outputBufferLeft, sampleCount); // moved out of render
    protectYourEars(outputBufferRight, sampleCount); // moved out of render
}

void Synth::midiMessage(uint8_t data0, uint8_t data1, uint8_t data2)
{
    switch (data0 & 0xF0)
    // first byte of the MIDI message is the status byte
    // consists of two parts: 1.) command 2.) channel number
    // switch(data0 & 0xF0) only looks at the four highest bits that make up the command, while skipping the four lowest bits that have the channel number
    {
        // Note off
        case 0x80 : // if the command is 0x80
            noteOff(data1 & 0x7F);
            break;

        // Note on
        case 0x90 : // if the command is 0x90
        { 
            uint8_t note = data1 & 0x7F; // value between 0 - 127
            uint8_t velo = data2 & 0x7F; // value between 0 - 127
            if (velo > 0) {
                noteOn(note, velo);
            }
            else {
                noteOff(note);
            }
            break;
        }
        
        // Pitch Bend
        case 0xE0 :
            pitchBend = std::exp(-0.000014102f * float(data1 + 128 * data2 - 8192));
            break;
    }
}

float Synth::calcPeriod(int note) const
{
    float period = tune * std::exp(-0.05776226505f * float(note));
    while (period < 6.0f || (period * detune) < 6.0f) { period += period; }
    return period;
}

void Synth::noteOn(int note, int velocity) // registers the note number and velocity of the most recently pressed key
{
    voice.note = note;
    voice.updatePanning();
    // voice.velocity = velocity; // you forgot to add this, don't forget it again! Without this, the sound won't play

    // float freq = 440.0f * std::exp2(float(note - 69) / 12.0f); // formula for twelve-tone equal temperament
    float period = calcPeriod(note);
    
    // voice.osc.amplitude = (velocity / 127.0f) * 0.5f;
    // voice.osc.period = sampleRate / freq;
    // voice.osc.reset();
    // voice.osc.inc = freq / sampleRate;
    // voice.osc.freq = freq;
    // voice.osc.sampleRate = sampleRate;
    // voice.osc.reset();
    // voice.osc.freq = 261.63f;
    // voice.osc.sampleRate = sampleRate;
    // voice.osc.phaseOffset = 0.0f;
    // voice.osc.reset();
    // voice.env.level = 1.0f;
    // voice.env.multiplier = envDecay;
    // voice.env.target = 0.2f;
    
    // activate first oscillator
    // voice.osc1.period = sampleRate / freq;

    // voice.period = sampleRate / freq;
    voice.period = period;
    voice.osc1.amplitude = (velocity / 127.0f) * 0.5f;
    voice.osc2.amplitude = voice.osc1.amplitude * oscMix;

    // voice.osc1.reset();
    
    // activate second oscillator
    // voice.osc2.period = voice.osc1.period * detune;
    // voice.osc2.amplitude = voice.osc1.amplitude * oscMix;
    // voice.osc2.reset();
    
    Envelope& env = voice.env;
    env.attackMultiplier = envAttack;
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = envRelease;
    env.attack();
    
    // env.level = 1.0f;
    // env.target = env.sustainLevel;
    // env.target = 20.0f;
    // env.multiplier = env.decayMultiplier;
}

void Synth::noteOff(int note) // voice.note variable is cleared only if the key that was released is for the same note
{
    if (voice.note == note) {
        // voice.note = 0;
        voice.release();
        // voice.velocity = 0;
    }
}
