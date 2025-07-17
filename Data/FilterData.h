/*
  ==============================================================================

    FilterData.h
    Created: 15 Dec 2024 9:28:28pm
    Author:  majab

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class FilterData
{
public:
    void prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels);
    void process(juce::AudioBuffer<float>& buffer);
    void updateParameters(int filterType, float baseCutoff, float resonance, float envValue = 0.0f);
    void reset();
    float processSample(int channel, float inputSample);

private:
    juce::dsp::StateVariableTPTFilter<float> filter;
    bool isPrepared{ false };

};
