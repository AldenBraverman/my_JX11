/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h> // necessary to pull in all JUCE modules
#include "Synth.h"
#include "Preset.h"

/*
 define new namespace containing a juce::ParameterID object for each of the 26 different parameter definition
 */
namespace ParameterID
{
    #define PARAMETER_ID(str) const juce::ParameterID str(#str, 1);
    
    PARAMETER_ID(oscMix)
    PARAMETER_ID(oscTune)
    PARAMETER_ID(oscFine)
    PARAMETER_ID(glideMode)
    PARAMETER_ID(glideRate)
    PARAMETER_ID(glideBend)
    PARAMETER_ID(filterFreq)
    PARAMETER_ID(filterReso)
    PARAMETER_ID(filterEnv)
    PARAMETER_ID(filterLFO)
    PARAMETER_ID(filterVelocity)
    PARAMETER_ID(filterAttack)
    PARAMETER_ID(filterDecay)
    PARAMETER_ID(filterSustain)
    PARAMETER_ID(filterRelease)
    PARAMETER_ID(envAttack)
    PARAMETER_ID(envDecay)
    PARAMETER_ID(envSustain)
    PARAMETER_ID(envRelease)
    PARAMETER_ID(lfoRate)
    PARAMETER_ID(vibrato)
    PARAMETER_ID(noise)
    PARAMETER_ID(octave)
    PARAMETER_ID(tuning)
    PARAMETER_ID(outputLevel)
    PARAMETER_ID(polyMode)
    PARAMETER_ID(panningLevel)

    #undef PARAMETER_ID
}

//==============================================================================
/**
*/
class My_JX11AudioProcessor  : public juce::AudioProcessor, // My_JX11AudioProcessor class extends juce::AudioProcessor
                               private juce::ValueTree::Listener // also need to inherit listener
{
public:
    //==============================================================================
    My_JX11AudioProcessor();
    ~My_JX11AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // Editor class that handles UI can access APVTS
    // Call createParameterLayout, inside this method is where we instantiate all the AudioParameterFloat and AudioParameterChoice objects
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters",
                                               createParameterLayout() };

private:
    // Split AudioBuffer into smaller pieces manage timing of MIDI messages
    void splitBufferByEvents(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void handleMIDI(uint8_t data0, uint8_t data1, uint8_t data2);
    void render(juce::AudioBuffer<float>& buffer, int sampleCount, int bufferOffset);
    Synth synth; // private member variable
    
    /*
     Create one variable for each paramter
     all these variables are pointers, actual paramter objects are owned by AudioProcessorValueTreeState (APVTS)
     */
    juce::AudioParameterFloat* oscMixParam;
    juce::AudioParameterFloat* oscTuneParam;
    juce::AudioParameterFloat* oscFineParam;
    juce::AudioParameterChoice* glideModeParam;
    juce::AudioParameterFloat* glideRateParam;
    juce::AudioParameterFloat* glideBendParam;
    juce::AudioParameterFloat* filterFreqParam;
    juce::AudioParameterFloat* filterResoParam;
    juce::AudioParameterFloat* filterEnvParam;
    juce::AudioParameterFloat* filterLFOParam;
    juce::AudioParameterFloat* filterVelocityParam;
    juce::AudioParameterFloat* filterAttackParam;
    juce::AudioParameterFloat* filterDecayParam;
    juce::AudioParameterFloat* filterSustainParam;
    juce::AudioParameterFloat* filterReleaseParam;
    juce::AudioParameterFloat* envAttackParam;
    juce::AudioParameterFloat* envDecayParam;
    juce::AudioParameterFloat* envSustainParam;
    juce::AudioParameterFloat* envReleaseParam;
    juce::AudioParameterFloat* lfoRateParam;
    juce::AudioParameterFloat* vibratoParam;
    juce::AudioParameterFloat* noiseParam;
    juce::AudioParameterFloat* octaveParam;
    juce::AudioParameterFloat* tuningParam;
    juce::AudioParameterFloat* outputLevelParam;
    juce::AudioParameterChoice* polyModeParam;
    juce::AudioParameterFloat* panningLevelParam;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout(); // declare method
    
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override
    {
        // when the user or host changes a parameter, valueTreePropertyChanged sets the parametersChanged boolean to true
        DBG("parameter changed");
        parametersChanged.store(true);
    }
    
    std::atomic<bool> parametersChanged { false }; // thread safe atomic variable
    
    void update();
    
    void createPrograms(); // fills presets vector
    std::vector<Preset> presets; // includes all Preset objects
    int currentProgram; // index of currently selected preset
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (My_JX11AudioProcessor)
};
