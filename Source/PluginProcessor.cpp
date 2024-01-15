/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
My_JX11AudioProcessor::My_JX11AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect // conditionals corresponding to options from the Projucer - defines can be found in JUCE Library Code/JucePluginDefines.h
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

My_JX11AudioProcessor::~My_JX11AudioProcessor()
{
}

//==============================================================================
const juce::String My_JX11AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool My_JX11AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool My_JX11AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool My_JX11AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double My_JX11AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int My_JX11AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int My_JX11AudioProcessor::getCurrentProgram()
{
    return 0;
}

void My_JX11AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String My_JX11AudioProcessor::getProgramName (int index)
{
    return {};
}

void My_JX11AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}


//==============================================================================
void My_JX11AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    synth.allocateResources(sampleRate, samplesPerBlock); // lets Synth object react to changes in sample rate or maximum block size
    reset();
}

void My_JX11AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    synth.deallocateResources(); // lets Synth object react to changes in sample rate or maximum block size
}

void My_JX11AudioProcessor::reset()
{
    synth.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool My_JX11AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void My_JX11AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) // audio callback
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    splitBufferByEvents(buffer, midiMessages);
    /*
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
    
    
    for (const auto metadata : midiMessages) {
        // if this is a Note On event, start the note
        // if this is a Note Off event, stop the note
    }
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        // output audio for the playing notes
    }
     */
    
}


// Split AudioBuffer into smaller pieces manage timing of MIDI messages
// Juce automatically sorts MIDI messages by their timestamp
void My_JX11AudioProcessor::splitBufferByEvents(juce::AudioBuffer<float> &buffer,
                                                juce::MidiBuffer &midiMessages)
{
    //
    int bufferOffset = 0;
    
    for (const auto metadata : midiMessages) { // For every MIDI event in midiMessages
        // Render the audio that happens before this event (if any)
        int samplesThisSegment = metadata.samplePosition - bufferOffset; // metadata.samplePosition = midi timestamp
        if (samplesThisSegment > 0) {
            render(buffer, samplesThisSegment, bufferOffset); // Process audio up to this event's timestamp
            bufferOffset += samplesThisSegment;
        }
        
        // Manage the midi event, also ignore 'sysex' MIDI messages
        if(metadata.numBytes <= 3) {
            uint8_t data1 = (metadata.numBytes >= 2) ? metadata.data[1] : 0;
            uint8_t data2 = (metadata.numBytes == 3) ? metadata.data[2] : 0;
            handleMIDI(metadata.data[0], data1, data2);
        }
        
    }
    
    // Render the audio after the last MIDI event - if there were no MIDI events at all, render the entire buffer
    int samplesLastSegment = buffer.getNumSamples() - bufferOffset;
    if (samplesLastSegment > 0) {
        render(buffer, samplesLastSegment, bufferOffset);
    }
    
    midiMessages.clear();
}

void My_JX11AudioProcessor::handleMIDI(uint8_t data0, uint8_t data1, uint8_t data2)
{
    // Print midi message
    // char s[16];
    // snprintf(s, 16, "%02hhX %02hhX %02hhX", data0, data1, data2);
    // DBG(s);
    synth.midiMessage(data0, data1, data2); // passes midi message to the Synth class
}

void My_JX11AudioProcessor::render(juce::AudioBuffer<float> &buffer, int sampleCount, int bufferOffset)
{
    // Naked float* pointer instead of juce::AudioBuffer, better for porting code
    // Pass array of two float* pointers to Synth, one for left and one for right
    // If Synth is configured to run in mono, only first pointer is used and the second will be nullptr
    float* outputBuffers[2] = { nullptr, nullptr };

    // call buffer.getWritePointer() and pass in the channel number to get a pointer to the audio data inside an AudioBuffer object
    // bufferOffset added because AudioBuffer is split up based on timestamps of MIDI events
    outputBuffers[0] = buffer.getWritePointer(0) + bufferOffset;
    if (getTotalNumOutputChannels() > 1) // for stereo plugins
    {
        outputBuffers[1] = buffer.getWritePointer(1) + bufferOffset;
    }

    synth.render(outputBuffers, sampleCount);
}

//==============================================================================
bool My_JX11AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* My_JX11AudioProcessor::createEditor()
{
    return new My_JX11AudioProcessorEditor(*this);
}

//==============================================================================
void My_JX11AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void My_JX11AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new My_JX11AudioProcessor();
}