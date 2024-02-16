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
}

void Synth::render(float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];

    // Loop through smaples in buffer
    // sampleCount is the number of samples we need to render, if there were midi messages, sampleCount will be less than the total number of samples in the block
    for (int sample = 0; sample < sampleCount; ++sample) {
        // Get next output from noise gen
        float noise = noiseGen.nextValue() * noiseMix; // added noiseMix control parameter

        // check if voice.note is not 0 (a key is pressed - synth recieved noteOn but not noteOff)
        float output = 0.0f;
        if (voice.note > 0) {
            // Noise value multiplied by velocity
            // output = noise * (voice.velocity / 127.0f) * 0.5f; // Multiplying the output by 0.5 = 6 dB reduction in gain
            output = voice.render(noise);// +noise; // instead of using output of noise gen, now we ask VOice object to produce next value for sin wave - update, added noise mix parameter - update, envelope affects noise now
        }

        protectYourEars(outputBufferLeft, sampleCount);
        protectYourEars(outputBufferRight, sampleCount);

        // Write output value into audio buffers with mono/stereo logic
        outputBufferLeft[sample] = output;
        if (outputBufferRight != nullptr) {
            outputBufferRight[sample] = output;
        }
    }
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
    }
}

void Synth::noteOn(int note, int velocity) // registers the note number and velocity of the most recently pressed key
{
    voice.note = note;
    // voice.velocity = velocity; // you forgot to add this, don't forget it again! Without this, the sound won't play

    float freq = 440.0f * std::exp2(float(note - 69) / 12.0f); // formula for twelve-tone equal temperament
    
    voice.osc.amplitude = (velocity / 127.0f) * 0.5f;
    voice.osc.period = sampleRate / freq;
    voice.osc.reset();
    // voice.osc.inc = freq / sampleRate;
    // voice.osc.freq = freq;
    // voice.osc.sampleRate = sampleRate;
    // voice.osc.reset();
    // voice.osc.freq = 261.63f;
    // voice.osc.sampleRate = sampleRate;
    // voice.osc.phaseOffset = 0.0f;
    // voice.osc.reset();
    voice.env.level = 1.0f;
    voice.env.multiplier = envDecay;
    voice.env.target = 0.2f;
}

void Synth::noteOff(int note) // voice.note variable is cleared only if the key that was released is for the same note
{
    if (voice.note == note) {
        voice.note = 0;
        // voice.velocity = 0;
    }
}
