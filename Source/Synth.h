/*
  ==============================================================================

    Synth.h
    Created: 12 Jan 2024 7:38:21pm
    Author:  Alden

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Voice.h" // import Voice class
#include "NoiseGenerator.h" // import NoiseGenerator class

class Synth // Synth has methods to reset its state, render current block of audio, and to manage MIDI messages
{
public:
    Synth();

    void allocateResources(double sampleRate, int samplesPerBlock); // analogous to prepareToPlay
    void deallocateResources(); // analogous to releaseResources
    void reset();
    void render(float** outputBuffers, int sampleCount);
    void midiMessage(uint8_t data0, uint8_t data1, uint8_t data2);
    void controlChange(uint8_t data1, uint8_t data2);

    float volumeTrim;
    
    float detune;

    float tune;
    
    float oscMix; // public variable because it will be filled in by the audio processor

    float pwmDepth;
    
    float noiseMix;
    
    float envAttack;
    float envDecay;
    float envSustain;
    float envRelease;
    
    static constexpr int MAX_VOICES = 8;
    int numVoices;
    
    // float outputLevel;
    
    // juce::LinearSmoothedValue does linear interpolation between the previous value and the new value
    juce::LinearSmoothedValue<float> outputLevelSmoother;
    
    float velocitySensitivity;
    bool ignoreVelocity;
    
    const int LFO_MAX = 32;
    float lfoInc;
    float vibrato;
    
    int glideMode;
    float glideRate;
    float glideBend;
    
    float filterKeyTracking;
    
    float filterQ;
    
    float filterLFODepth;
    
    float filterAttack, filterDecay, filterSustain, filterRelease;
    float filterEnvDepth;
    
private:
    float sampleRate;
    // Voice voice; // Set up Voice class // this is just one voice for monophonic
    std::array<Voice, MAX_VOICES> voices; // array of voices for polyphony
    void noteOn(int note, int velocity);
    void noteOff(int note);
    NoiseGenerator noiseGen; // Set up NoiseGenerator class
    float calcPeriod(int v, int note) const;
    float pitchBend;
    void startVoice(int v, int note, int velocity); // v = index of voice to use
    int findFreeVoice() const; // voice stealing
    bool sustainPedalPressed;
    void restartMonoVoice(int note, int velocity);
    void shiftQueuedNotes();
    int nextQueuedNote();
    
    void updateLFO();
    int lfoStep;
    float lfo;
    float modWheel;
    
    int lastNote;
    
    inline void updatePeriod(Voice& voice)
    {
        voice.osc1.period = voice.period * pitchBend;
        voice.osc2.period = voice.osc1.period * detune;
    }
    
    bool isPlayingLegatoStyle() const;
    
    float resonanceCtl;
    
    float pressure;
    
    float filterCtl;
    
    float filterZip;
};
