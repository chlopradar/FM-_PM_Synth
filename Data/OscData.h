/*
  ==============================================================================

    OscData.h
    Created: 2 Dec 2024 11:47:42pm
    Author:  majab

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class OscData
{
public:
    void prepareToPlay(juce::dsp::ProcessSpec& spec);
    void setWaveType(int choice);
    void setCoarse(float newCoarse) { coarse = newCoarse; }
    void setFine(float newFine) { fine = newFine; }
    void setGain(float newGain) { gain = newGain; }
    void setBaseFrequency(float freq);
    void setBaseFreqParams(float newBaseFreq, float newCoarse, float newFine);

    float getModulatedSample(float modulation, float modEnv = 1.0f);
    void resetPhase() { currentPhase = 0.0f; }
    float getPhase() const { return currentPhase; }

    float getCoarse() const { return coarse; }
    float getFine() const { return fine; }
    float getGain() const { return gain; }
    void resetModState() noexcept
    {
        modulationHP = 0.0f;
        prevModulation = 0.0f;
    }

private:
    void updatePhaseIncrement(float freq);

    float noteBaseFrequency = 0.0f;
    float coarse = 1.0f;
    float fine = 0.0f;
    float gain = 0.0f;

    double sampleRate = 48000.0;
    int waveType = 0;

    float modulationScale = 0.05f; 
    float currentPhase = 0.0f;
    float phaseIncrement = 0.0f;

    float modulationHP = 0.0f;  // stan filtra HPF
    float prevModulation = 0.0f;  // poprzednia próbka modulacji
};

