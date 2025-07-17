/*
  ==============================================================================

    OscData.cpp
    Created: 2 Dec 2024 11:47:42pm
    Author:  majab

  ==============================================================================
*/

#include "OscData.h"

void OscData::prepareToPlay(juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    // domsylnie brak fazy zresetuj na start
    currentPhase = 0.0f;
}

void OscData::setWaveType(int choice)
{
    waveType = choice;
}

void OscData::setBaseFrequency(float freq)
{
    noteBaseFrequency = freq;
}

void OscData::updatePhaseIncrement(float freq)
{
    // przelicz hz na increment fazy (radiany/sample)
    phaseIncrement = (freq / (float)sampleRate) * juce::MathConstants<float>::twoPi;
}

float OscData::getModulatedSample(float modulation, float modEnv)
{
    // DC-bloker aby zapobiec zmianom czestotliwosci
    float freq = juce::MathConstants<float>::twoPi * 40;
    const float alpha = (sampleRate - freq )/freq; 
    modulationHP = alpha * (modulationHP + modulation - prevModulation);
    prevModulation = modulation;

    // skala modulacji (indeks FM)
    float modulationIndex = modulationScale;

    // aktualizacja fazy z uwzgl odfiltrowanej modulacji
    float fm = modulationHP * modulationIndex;
    currentPhase += phaseIncrement + fm;

    // zawijanie fazy do 0, 2pi
    currentPhase = std::fmod(currentPhase, juce::MathConstants<float>::twoPi);
    if (currentPhase < 0.0f)
        currentPhase += juce::MathConstants<float>::twoPi;
     
    

    // generacja probki zgodnie z typem fali
    float sample = 0.0f;
    switch (waveType)
    {
    case 0: // sine
        sample = std::sin(currentPhase);
        break;
    case 1: // saw
        sample = 1.0f - 2.0f * (currentPhase / juce::MathConstants<float>::twoPi);
        break;
    case 2: // square
        sample = (currentPhase < juce::MathConstants<float>::pi) ? 1.0f : -1.0f;
        break;
    case 3: // triangle
        sample = (2.0f / juce::MathConstants<float>::pi) * std::asin(std::sin(currentPhase));
        break;
    default:
        sample = std::sin(currentPhase);
        break;
    }

    // zwracamy probke pomnożoną przez gain i obwiednie
    return sample * gain * modEnv;
}



void OscData::setBaseFreqParams(float newBaseFreq, float newCoarse, float newFine)
{
    noteBaseFrequency = newBaseFreq;
    coarse = newCoarse;
    fine = newFine;
    float freq = noteBaseFrequency * (coarse + fine * 0.001f);
    updatePhaseIncrement(freq);
}
