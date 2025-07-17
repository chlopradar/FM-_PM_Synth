/*
  ==============================================================================

    SynthVoice.cpp
    Created: 3 Oct 2024 12:07:58pm
    Author:  Radek

  ==============================================================================
*/

#include "SynthVoice.h"
#include "Data/FMAlgorithmRouter.h"

bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<juce::SynthesiserSound*>(sound) != nullptr;
}
void SynthVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition)
{
    baseFrequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    osc1.setBaseFrequency(baseFrequency);
    osc2.setBaseFrequency(baseFrequency);
    osc3.setBaseFrequency(baseFrequency);
    osc4.setBaseFrequency(baseFrequency);

    osc1.resetPhase();
    osc2.resetPhase();
    osc3.resetPhase();
    osc4.resetPhase();
    
    osc1.resetModState();
    osc2.resetModState();
    osc3.resetModState();
    osc4.resetModState();

    adsr1.noteOn();
    adsr2.noteOn();
    adsr3.noteOn();
    adsr4.noteOn();

    modAdsr.noteOn();
}
void SynthVoice::stopNote(float velocity, bool allowTailOff)
{
    adsr1.noteOff();
    adsr2.noteOff();
    adsr3.noteOff();
    adsr4.noteOff();

    modAdsr.noteOff();

    if (!allowTailOff || !adsr1.isActive())
        clearCurrentNote();
}
void SynthVoice::controllerMoved(int controllerNumber, int newControllerValue)
{

}
void SynthVoice::pitchWheelMoved(int newPitchWheelValue)
{

}
void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = outputChannels;

    osc1.prepareToPlay(spec);
    osc2.prepareToPlay(spec);
    osc3.prepareToPlay(spec);
    osc4.prepareToPlay(spec);

    adsr1.setSampleRate(sampleRate);
    adsr2.setSampleRate(sampleRate);
    adsr3.setSampleRate(sampleRate);
    adsr4.setSampleRate(sampleRate);

    filter.prepareToPlay(sampleRate, samplesPerBlock, outputChannels);
    modAdsr.setSampleRate(sampleRate);
    gain.prepare(spec);

    isPrepared = true;
}

void SynthVoice::updateAdsr(int index, const float attack, const float decay, const float sustain, const float release)
{
    switch (index)
    {
    case 1: adsr1.updateADSR(attack, decay, sustain, release); break;
    case 2: adsr2.updateADSR(attack, decay, sustain, release); break;
    case 3: adsr3.updateADSR(attack, decay, sustain, release); break;
    case 4: adsr4.updateADSR(attack, decay, sustain, release); break;
    default: jassertfalse;
    }
}


void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
    int startSample,
    int numSamples)
{
    jassert(isPrepared);
    if (!isVoiceActive())
        return;

    synthBuffer.setSize(outputBuffer.getNumChannels(), numSamples, false, false, true);
    synthBuffer.clear();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // dla kazdego osc kolejna probka z adsr'a
        float env1 = adsr1.getNextSample();
        float env2 = adsr2.getNextSample();
        float env3 = adsr3.getNextSample();
        float env4 = adsr4.getNextSample();

        // aktualny numer algorytmu
        int algorithmIndex = currentAlgorithm; 

        // wykonanie funkcji algorytmu sterujacej routingiem modulacji
        float oscOutput = FMAlgorithmRouter::processAlgorithm(algorithmIndex,
            osc1, osc2, osc3, osc4,
            env1, env2, env3, env4);

        float processed = oscOutput;

        if (filterEnabled)
        {
            float modEnvValue = modAdsr.getNextSample(); // 0-1
            filter.updateParameters(currentFilterType, currentCutoff,
                currentResonance, modEnvValue);

            //for (int ch = 0; ch < synthBuffer.getNumChannels(); ++ch)
                processed = filter.processSample(0, processed);
        }

        // zapis do bufora
        for (int ch = 0; ch < synthBuffer.getNumChannels(); ++ch)
            synthBuffer.setSample(ch, sample, processed);

    }

    DBG("PHASE 1 : " << osc1.getPhase());
    DBG("PHASE 2 : " << osc2.getPhase());
    DBG("PHASE 3 : " << osc3.getPhase());
    DBG("PHASE 4 : " << osc3.getPhase());

    juce::dsp::AudioBlock<float> audioBlock{ synthBuffer };
    gain.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
    gain.setGainLinear(0.2f);

    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
    {
        outputBuffer.addFrom(channel, startSample, synthBuffer, channel, 0, numSamples);
    }

    if (!adsr1.isActive())
        clearCurrentNote();
}

void SynthVoice::updateFilter(int newFilterType, float newCutoff, float newResonance)
{
    // zapisz parametry filtra w obiekcie głosu
    currentFilterType = newFilterType;
    currentCutoff = newCutoff;
    currentResonance = newResonance;
}

void SynthVoice::updateModAdsr(const float attack, const float decay,
    const float sustain, const float release)
{
    modAdsr.updateADSR(attack, decay, sustain, release);
}
OscData& SynthVoice::getOscillator(int index)
{
    switch (index)
    {
    case 1: return osc1;
    case 2: return osc2;
    case 3: return osc3;
    case 4: return osc4;
    default:
        jassertfalse; 
        return osc1;  // domyslnie zwraca osc1
    }
}
