// =================== PluginProcessor.cpp ===================

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Data/VocoderData.h"

//==============================================================================
FM_SYNTHAudioProcessor::FM_SYNTHAudioProcessor()
    : AudioProcessor(
        BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)   // wejscie stereo
        .withOutput("Output", juce::AudioChannelSet::stereo(), true) // wyjscie stereo 
    ),
    apvts(*this, nullptr, "Parameters", createParameters())
{
    synth.addSound(new SynthSound());
    synth.addVoice(new SynthVoice());
    //for (int i = 0; i < 8; ++i)
    //{
    //    synth.addVoice(new SynthVoice());
    //}

}

FM_SYNTHAudioProcessor::~FM_SYNTHAudioProcessor()
{
}

//==============================================================================
const juce::String FM_SYNTHAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FM_SYNTHAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool FM_SYNTHAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool FM_SYNTHAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double FM_SYNTHAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FM_SYNTHAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int FM_SYNTHAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FM_SYNTHAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String FM_SYNTHAudioProcessor::getProgramName(int index)
{
    return {};
}

void FM_SYNTHAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void FM_SYNTHAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);

    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            voice->prepareToPlay(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
        }
    }

    vocoder.prepareToPlay(sampleRate, samplesPerBlock);
}

void FM_SYNTHAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FM_SYNTHAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
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

void FM_SYNTHAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;



    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    // czyszczenie nieuzywanych kanalow
    for (int ch = totalNumInputChannels; ch < totalNumOutputChannels; ++ch)
        buffer.clear(ch, 0, numSamples);

    // pobranie sygnalu z mikrofonu do bufora
    juce::AudioBuffer<float> modBuffer;
    modBuffer.setSize(1, numSamples);
    if (totalNumInputChannels > 0)
        modBuffer.copyFrom(0, 0, buffer, 0, 0, numSamples);   // wejście-L -> modulator
    else
        modBuffer.clear();

    // konfiguracja wszystkich voices
    for (int v = 0; v < synth.getNumVoices(); ++v)
    {
        if (auto* voice = dynamic_cast<SynthVoice*> (synth.getVoice(v)))
        {
            // algorytm
            auto& algoParam = *apvts.getRawParameterValue("ALGORITHM");
            voice->setAlgorithm(static_cast<int> (algoParam.load()));

            // osc i adsr
            for (int oscIndex = 1; oscIndex <= 4; ++oscIndex)
            {
                // parametry osc
                auto& waveType = *apvts.getRawParameterValue("OSC" + std::to_string(oscIndex) + "WAVETYPE");
                auto& coarse = *apvts.getRawParameterValue("OSC" + std::to_string(oscIndex) + "COARSE");
                auto& fine = *apvts.getRawParameterValue("OSC" + std::to_string(oscIndex) + "FINE");
                auto& gain = *apvts.getRawParameterValue("OSC" + std::to_string(oscIndex) + "GAIN");

                auto& osc = voice->getOscillator(oscIndex);
                osc.setWaveType(static_cast<int> (waveType.load()));
                osc.setCoarse(coarse.load());
                osc.setFine(fine.load());
                osc.setGain(gain.load());
                osc.setBaseFreqParams(voice->getBaseFrequency(), coarse.load(), fine.load());

                // paramtery adsr
                auto& attack = *apvts.getRawParameterValue("OSC" + std::to_string(oscIndex) + "ATTACK");  
                auto& decay = *apvts.getRawParameterValue("OSC" + std::to_string(oscIndex) + "DECAY");   
                auto& sustain = *apvts.getRawParameterValue("OSC" + std::to_string(oscIndex) + "SUSTAIN");  
                auto& release = *apvts.getRawParameterValue("OSC" + std::to_string(oscIndex) + "RELEASE");  

                // zmiana na sek
                float aSec = attack.load() * 0.001f;                
                float dSec = decay.load() * 0.001f;
                float rSec = release.load() * 0.001f;
                float sAmp = juce::Decibels::decibelsToGain(sustain.load()); 

                voice->updateAdsr(oscIndex, aSec, dSec, sAmp, rSec);
            }

            // paramtery filtra
            auto& filterType = *apvts.getRawParameterValue("FILTERTYPE");
            auto& cutoff = *apvts.getRawParameterValue("FILTERFREQ");
            auto& resonance = *apvts.getRawParameterValue("FILTERRES");
            voice->updateFilter(filterType.load(), cutoff.load(), resonance.load());

            auto& filterOn = *apvts.getRawParameterValue("FILTERON");
            voice->setFilterEnabled(filterOn.load());

            // paramtery adsra filtra
            auto& modAttack = *apvts.getRawParameterValue("MODATTACK");  
            auto& modDecay = *apvts.getRawParameterValue("MODDECAY");   
            auto& modSustain = *apvts.getRawParameterValue("MODSUSTAIN"); 
            auto& modRelease = *apvts.getRawParameterValue("MODRELEASE"); 

            float mASec = modAttack.load() * 0.001f;       
            float mDSec = modDecay.load() * 0.001f;
            float mRSec = modRelease.load() * 0.001f;
            float mSSustain = modSustain.load() / 100.0f;   

            voice->updateModAdsr(mASec, mDSec, mSSustain, mRSec);
        }
    }

    // wygenerowanie sygnalu
    juce::AudioBuffer<float> carrierBuffer;
    carrierBuffer.setSize(totalNumOutputChannels, numSamples);
    carrierBuffer.clear();
    synth.renderNextBlock(carrierBuffer, midiMessages, 0, numSamples);

    // paramtery vocodera
    vocoder.setSmoothingFactor(*apvts.getRawParameterValue("SMOOTHFAC"));

    bool vocoderEnabled = *apvts.getRawParameterValue("VOCODER");
    if (vocoderEnabled)
    {
        vocoder.process(modBuffer, carrierBuffer, buffer);      // OUT -> buffer
    }
    else
    {
        for (int ch = 0; ch < totalNumOutputChannels; ++ch)
            buffer.copyFrom(ch, 0, carrierBuffer, ch, 0, numSamples);
    }

    updateOscilloscopeBuffer(buffer);
}


//==============================================================================
bool FM_SYNTHAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FM_SYNTHAudioProcessor::createEditor()
{
    return new FM_SYNTHAudioProcessorEditor(*this);
}

//==============================================================================
void FM_SYNTHAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void FM_SYNTHAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FM_SYNTHAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout FM_SYNTHAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;


    //OSC
    for (int i = 1; i <= 4; ++i)
    {
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            "OSC" + std::to_string(i) + "WAVETYPE", "Osc " + std::to_string(i) + " Wave Type",
            juce::StringArray{ "Sine", "Saw", "Square", "Triangle" }, 0));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "OSC" + std::to_string(i) + "COARSE", "Osc " + std::to_string(i) + " Coarse",
            juce::NormalisableRange<float>{0.5f, 16.0f, 0.5f}, 1.0f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "OSC" + std::to_string(i) + "FINE", "Osc " + std::to_string(i) + " Fine",
            juce::NormalisableRange<float>{0.0f, 1000.0f, 0.1f}, 0.0f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            "OSC" + std::to_string(i) + "GAIN", "Osc " + std::to_string(i) + " Gain",
            juce::NormalisableRange<float>{0.0f, 1.0f, 0.01f}, 0.0f));
    }

    //ADSR
    for (int i = 1; i <= 4; ++i)
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC" + std::to_string(i) + "ATTACK",
            "Osc " + std::to_string(i) + " Attack", juce::NormalisableRange<float>{0.0f, 2000.0f, 0.01f, 0.20f}, 0.0f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC" + std::to_string(i) + "DECAY",
            "Osc " + std::to_string(i) + " Decay", juce::NormalisableRange<float>{1.0f, 6000.0f, 0.1f, 0.20f}, 1.0f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC" + std::to_string(i) + "SUSTAIN",
            "Osc " + std::to_string(i) + " Sustain", juce::NormalisableRange<float>{-60.0f, 0.0f, 0.1f}, 0.0f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>("OSC" + std::to_string(i) + "RELEASE",
            "Osc " + std::to_string(i) + " Release", juce::NormalisableRange<float>{1.0f, 6000.0f, 0.1f, 0.20f}, 1.0f));
    }


    //FILTER ADSR 
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MODATTACK", "Mod Attack", juce::NormalisableRange<float>{0.0f, 2000.0f, 0.01f, 0.20f}, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MODDECAY", "Mod Decay", juce::NormalisableRange<float>{1.0f, 6000.0f, 0.1f, 0.20f}, 250.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MODSUSTAIN", "Mod Sustain", juce::NormalisableRange<float>{0.0f, 100.0f, 0.1f}, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MODRELEASE", "Mod Release", juce::NormalisableRange<float>{1.0f, 6000.0f, 0.1f, 0.20f}, 1.0f));

    //FILTER
    params.push_back(std::make_unique<juce::AudioParameterChoice>("FILTERTYPE", "Filter Type", juce::StringArray{ "Low-pass", "Band-pass", "High-pass" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERFREQ", "Filter Freq", juce::NormalisableRange<float> {20.0f, 20000.0f, 1.0f, 0.6f}, 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTERRES", "Filter Resonance", juce::NormalisableRange<float> {1.0f, 10.0f, 0.1f}, 2.5f));

    params.push_back(std::make_unique<juce::AudioParameterBool>("VOCODER", "Vocoder", false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("SMOOTHFAC", "Smoothing Factor", juce::NormalisableRange<float> {0.01f, 0.5f, 0.01f}, 0.01f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "ALGORITHM",               
        "Algorithm",               
        juce::StringArray{ "Alg 1", "Alg 2", "Alg 3", "Alg 4", "Alg 5", "Alg 6", "Alg 7", "Alg 8" },
        0));                    

    params.push_back(std::make_unique<juce::AudioParameterBool>("FILTERON", "Filter On", false));

    return { params.begin(), params.end() };
}


float FM_SYNTHAudioProcessor::getCurrentFrequency() const
{
    // zwracanie czestotliwosci pierwszego glosu
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
        {
            if (voice->isVoiceActive())
                return voice->getBaseFrequency();
        }
    }
    // jak nie jest aktywny to A4
    return 440.0f;
}



