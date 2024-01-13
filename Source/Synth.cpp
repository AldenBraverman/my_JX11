/*
  ==============================================================================

    Synth.cpp
    Created: 12 Jan 2024 7:38:21pm
    Author:  Alden

  ==============================================================================
*/

#include "Synth.h"

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
}

void Synth::render(float** outputBuffers, int sampleCount)
{
    // do nothing yet
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
}

void Synth::noteOff(int note) // voice.note variable is cleared only if the key that was released is for the same note
{
    if (voice.note == note) {
        voice.note = 0;
        voice.velocity = 0;
    }
}