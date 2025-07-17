/*
  ==============================================================================

    FilterData.cpp
    Created: 15 Dec 2024 9:28:28pm
    Author:  majab

  ==============================================================================
*/

#include "FilterData.h"

void FilterData::prepareToPlay(double sampleRate, int samplesPerBlock, int numChannels)
{
    filter.reset();

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = numChannels;

    filter.prepare(spec);

    isPrepared = true;
}

void FilterData::process(juce::AudioBuffer<float>& buffer) 
{
    jassert(isPrepared);

    juce::dsp::AudioBlock<float> block { buffer };
    filter.process(juce::dsp::ProcessContextReplacing<float> { block });
}

void FilterData::updateParameters(int   filterType,
    float baseCutoff,
    float resonance,
    float envValue /* 0-1 */)
{
    using FType = juce::dsp::StateVariableTPTFilterType;
    filter.setType(static_cast<FType> (filterType));

    float modCutoff = baseCutoff + envValue * (20000.0f - baseCutoff);

    modCutoff = juce::jlimit(20.0f, 20000.0f, modCutoff);
    filter.setCutoffFrequency(modCutoff);
    filter.setResonance(resonance);
}

void FilterData::reset()
{
    filter.reset();
}

float FilterData::processSample(int channel, float inputSample)
{
    return filter.processSample(channel, inputSample);
}