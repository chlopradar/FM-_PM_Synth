/*
  ==============================================================================

    VocoderData.h
    Created: 23 Mar 2025 6:11:13pm
    Author:  majab

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <array>

class VocoderData {
public:
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void process(const juce::AudioBuffer<float>& modBuffer,
        const juce::AudioBuffer<float>& carrierBuffer,
        juce::AudioBuffer<float>& outputBuffer);

    void setSmoothingFactor(float newFactor) noexcept { smoothingFactor = newFactor; }

private:
    static constexpr int numBands = 24;
    std::array<juce::dsp::IIR::Filter<float>, numBands> modFilters;        // filtry pasmowe dla modulatora
    std::array<juce::dsp::IIR::Filter<float>, numBands> carrierFiltersLeft;  // filtry pasmowe dla noœnego (kanal L)
    std::array<juce::dsp::IIR::Filter<float>, numBands> carrierFiltersRight; // filtry pasmowe dla noœnego (kanal R)
    std::array<float, numBands> bandEnvelopes{};  // obwiednie (gain) dla ka¿dego pasma

    // smoothing do wygladzania ¿eby nie by³o pop-ow
    std::array<float, numBands> previousEnvelopes{};
    float smoothingFactor{ 0.01f };
};
