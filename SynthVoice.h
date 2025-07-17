/*
  ==============================================================================

    SynthVoice.h
    Created: 3 Oct 2024 12:07:58pm
    Author:  Radek

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "SynthSound.h"
#include "Data/OscData.h"
#include "Data/AdsrData.h"
#include "Data/FilterData.h"

class SynthVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels);
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    void updateAdsr(int index, const float attack, const float decay, const float sustain, const float release);
    void updateFilter(int newFilterType, float newCutoff, float newResonance);
    void updateModAdsr(const float attack, const float decay, const float sustain, const float release);
    OscData& getOscillator(int index);

    void setAlgorithm(int newAlgorithmIndex) { currentAlgorithm = newAlgorithmIndex; }

    float getBaseFrequency() const { return baseFrequency; }
    void setFilterEnabled(bool enabled) { filterEnabled = enabled; }

private:

    juce::AudioBuffer<float> synthBuffer;

    OscData osc1, osc2, osc3, osc4; 
    float baseFrequency{ 0.0f };   
    AdsrData adsr1, adsr2, adsr3, adsr4;
    FilterData filter;
    AdsrData modAdsr;
    int currentFilterType{ 0 };      
    float currentCutoff{ 500.0f };  
    float currentResonance{ 1.0f };  

    juce::dsp::Gain<float> gain;

    int currentAlgorithm = 0;
    bool filterEnabled{ true };
    bool isPrepared{ false };
};
