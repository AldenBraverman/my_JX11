/*
  ==============================================================================

    Synth.cpp
    Created: 12 Jan 2024 7:38:21pm
    Author:  Alden

  ==============================================================================
*/

#include "Synth.h"
#include "Utils.h"

static const float ANALOG = 0.002f; // polyphony enhancement
static const int SUSTAIN = -1;

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
    // voice.reset(); the old monophonic way
    for (int v = 0; v < MAX_VOICES; ++v) {
        voices[v].reset();
    }
    noiseGen.reset();
    pitchBend = 1.0f;
    sustainPedalPressed = false;
    
    outputLevelSmoother.reset(sampleRate, 0.05);
}

void Synth::render(float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];
    
    // voice.osc1.period = voice.period * pitchBend; // FORGOT THESE LINES, NO SOUND WITHOUT THEM
    // voice.osc2.period = voice.osc1.period * detune;
    
    // Assign period plus any detuning and pitch bend to the voice's oscillators
    // done at start of block, done for all voices that are currently being rendered
    for (int v = 0; v < MAX_VOICES; ++v) {
        Voice& voice = voices[v];
        if (voice.env.isActive()) {
            voice.osc1.period = voice.period * pitchBend;
            voice.osc2.period = voice.osc1.period * detune;
        }
    }

    // Loop through samples in buffer
    // sampleCount is the number of samples we need to render, if there were midi messages, sampleCount will be less than the total number of samples in the block
    for (int sample = 0; sample < sampleCount; ++sample) {
        
        // Get next output from noise gen
        const float noise = noiseGen.nextValue() * noiseMix; // added noiseMix control parameter

        // check if voice.note is not 0 (a key is pressed - synth recieved noteOn but not noteOff)
        //float output = 0.0f;
        // separate output variables for the left and right channels
        float outputLeft = 0.0f;
        float outputRight = 0.0f;
        
        // Render all the active voices, the sample value produced by each Voice object is added to outputLeft/outputRight variables (which are initially zero)
        // Multiple voices get mixed together, by adding them up, the noise oscillator is shared by all voices
        for (int v = 0; v < MAX_VOICES; ++v) {
            Voice& voice = voices[v];
            if (voice.env.isActive()) {
                float output = voice.render(noise);
                outputLeft += output * voice.panLeft;
                outputRight += output * voice.panRight;
                
                float outputLevel = outputLevelSmoother.getNextValue();
                outputLeft *= outputLevel;
                outputRight *= outputLevel;
            }
        }
        
        /*
        if (voice.env.isActive()) { // originally was voice.note > 0
            // Noise value multiplied by velocity
            // output = noise * (voice.velocity / 127.0f) * 0.5f; // Multiplying the output by 0.5 = 6 dB reduction in gain
            // output = voice.render(noise);// +noise; // instead of using output of noise gen, now we ask VOice object to produce next value for sin wave - update, added noise mix parameter - update, envelope affects noise now
            // sample renders in stereo
            float output = voice.render(noise);
            outputLeft += output * voice.panLeft;
            outputRight += output * voice.panRight;
        }
        */
        
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
    
    // after the loop, if any voices has its envelope below the SILENCE level, voice is disabled
    for (int v = 0; v < MAX_VOICES; ++v) {
        Voice& voice = voices[v];
        if (!voice.env.isActive()) {
            voice.env.reset();
        }
    }
    
    /*
    if (!voice.env.isActive()) {
        voice.env.reset();
    }
    */
    
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
            
        // Control change
        case 0xB0 :
            controlChange(data1, data2);
            break;
    }
}

float Synth::calcPeriod(int v, int note) const
{
    // float period = tune * std::exp(-0.05776226505f * float(note));
    float period = tune * std::exp(-0.05776226505f * (float(note) + ANALOG * float(v))); // polyphony enhancement
    while (period < 6.0f || (period * detune) < 6.0f) { period += period; }
    return period;
}

void Synth::startVoice(int v, int note, int velocity) // copy of noteOn method from at chapter 10, v = index of voice to use
{
    float period = calcPeriod(v, note);
    
    Voice& voice = voices[v]; // new line from noteOn
    voice.period = period;
    voice.note = note;
    voice.updatePanning();
    
    // voice.osc1.amplitude = (velocity / 127.0f) * 0.5f;
    voice.osc1.amplitude = volumeTrim * velocity; 
    voice.osc2.amplitude = voice.osc1.amplitude * oscMix;
    
    Envelope& env = voice.env;
    env.attackMultiplier = envAttack;
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = envRelease;
    env.attack();
}

void Synth::noteOn(int note, int velocity) // registers the note number and velocity of the most recently pressed key
{
    // startVoice(0, note, velocity);
    
    int v = 0; // index of the voice to use (0 = mono voice)
    
    /*
    if (numVoices > 1) { // polyphonics
        v = findFreeVoice();
        // if no notes are playing yet, findFreeVoice returns 0
        // Otherwise, it returns the index of the next free voice
        // if all voices are in use, return the index of the voice with the smallest envelope level
    }
    */
    
    if(numVoices == 1) { // monophonic
        DBG("Synth::noteOn where numVoices == 1");
        if (voices[0].note > 0) { // legato-style playing
            shiftQueuedNotes();
            restartMonoVoice(note, velocity);
            return;
        }
    }
    
    
    // TESTING START
    if(numVoices == 2) {
        DBG("Synth::noteOn where numVoices == 2");
    }
    if(numVoices == 3) {
        DBG("Synth::noteOn where numVoices == 3");
    }
    if(numVoices == 4) {
        DBG("Synth::noteOn where numVoices == 4");
    }
    // TESTING END
    
    else { // polyphonic
        v = findFreeVoice();
    }
    
    startVoice(v, note, velocity);
    
    /*
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
    */
}

void Synth::noteOff(int note) // voice.note variable is cleared only if the key that was released is for the same note
{
    /*
    Voice& voice = voices[0];
    if (voice.note == note) {
        // voice.note = 0;
        voice.release();
        // voice.velocity = 0;
    }
    */
    
    if ((numVoices == 1) && (voices[0].note == note)) {
        int queuedNote = nextQueuedNote();
        if (queuedNote > 0) {
            restartMonoVoice(queuedNote, -1);
        }
    }
    
    for (int v = 0; v < MAX_VOICES; v++){
        if (voices[v].note == note) {
            // voices[v].release();
            // voices[v].note = 0;
            if (sustainPedalPressed) {
                voices[v].note = SUSTAIN;
            } else {
                voices[v].release();
                voices[v].note = 0;
            }
        }
    }
}

void Synth::controlChange(uint8_t data1, uint8_t data2)
{
    switch (data1) {
        default:
            if (data1 >= 0x78) {
                for (int v = 0; v < MAX_VOICES; ++v) {
                    voices[v].reset();
                }
                sustainPedalPressed = false;
            }
            break;
        // Sustain pedal
        case 0x40 :
            sustainPedalPressed = (data2 >= 64);

            if (!sustainPedalPressed) { 
                noteOff(SUSTAIN);
            }
            break;
    }
}

int Synth::findFreeVoice() const
{
    // method returns the index of the voice to use
    // loop finds the voice with the lowest envelope level
    // ignore voices in attack stage
    int v = 0;
    float l = 100.0f; // louder than any envelope!
    
    for (int i = 0; i < MAX_VOICES; ++i) {
        if (voices[i].env.level < 1 && !voices[i].env.isInAttack()) {
            l = voices[i].env.level;
            v = i;
        }
    }
    return v;
    
    /* TO-DO: Other voice stealing algorithms from page 243
     - steal the oldest playing note
     - steal the oldest note that isn't the lowest or highest pitched note currently being played
     - steal the note with the smallest velocity (or amplitude)
     - first try to steal notes that have been released already
     - if the same note was already playing, re-use that voice
     
     
     */
    
}

void Synth::restartMonoVoice(int note, int velocity)
{
    float period = calcPeriod(0, note);
    
    Voice& voice = voices[0];
    voice.period = period;
    
    voice.env.level += SILENCE + SILENCE;
    voice.note = note;
    voice.updatePanning();
}

void Synth::shiftQueuedNotes()
{
    for (int tmp = MAX_VOICES - 1; tmp > 0; tmp--) {
        voices[tmp].note = voices[tmp - 1].note;
    }
}

int Synth::nextQueuedNote()
{
    int held = 0;
    for (int v = MAX_VOICES - 1; v > 0; v--) {
        if (voices[v].note > 0) { held = v; }
    }
    
    if (held > 0) {
        int note = voices[held].note;
        voices[held].note = 0;
        return note;
    }
    
    return 0;
}
