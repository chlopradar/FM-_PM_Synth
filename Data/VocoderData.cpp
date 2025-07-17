#include "VocoderData.h"
#include <cmath>

void VocoderData::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // obliczamy 24 czêstotliwoœci œrodkowe rozmieszczone logarytmicznie miêdzy fmin a fmax
    std::array<float, numBands> centerFreqs;
    float fmin = 200.f;
    float fmax = 16000.f;
    for (int i = 0; i < numBands; ++i)
    {
        float fraction = static_cast<float>(i) / (numBands - 1);
        centerFreqs[i] = fmin * std::pow(fmax / fmin, fraction);
    }

    // uzywamy umiarkowanego Q zeby nie popowalo
    float Q = 10.0f; 

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;  // kazdy filtr w mono

    // init filtra poprzednich obwiedni
    previousEnvelopes.fill(0.0f);

    // init filtra dla kazdego pasma
    for (int band = 0; band < numBands; ++band)
    {
        auto coeff = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, centerFreqs[band], Q);

        modFilters[band].prepare(spec);
        modFilters[band].coefficients = coeff;
        modFilters[band].reset();

        carrierFiltersLeft[band].prepare(spec);
        carrierFiltersLeft[band].coefficients = coeff;
        carrierFiltersLeft[band].reset();

        carrierFiltersRight[band].prepare(spec);
        carrierFiltersRight[band].coefficients = coeff;
        carrierFiltersRight[band].reset();
    }
}

void VocoderData::process(const juce::AudioBuffer<float>& modBuffer,
    const juce::AudioBuffer<float>& carrierBuffer,
    juce::AudioBuffer<float>& outputBuffer)
{
    jassert(modBuffer.getNumChannels() > 0 && carrierBuffer.getNumChannels() >= 1);
    const int numSamples = modBuffer.getNumSamples();
    outputBuffer.clear();  // wyczysc bufor przed sumowaniem

    // modBuffer jest mono kanal 0 zawiera sygnal mikrofonu
    const float* modSignal = modBuffer.getReadPointer(0);

    // przetwarzamy ka¿dy z 24 pasm
    for (int band = 0; band < numBands; ++band)
    {
        // obliczamy obwiednie dla pasma – suma wartosci bezwzglednych po filtrowaniu
        float envSum = 0.0f;
        for (int i = 0; i < numSamples; ++i)
        {
            float filteredMod = modFilters[band].processSample(modSignal[i]);
            envSum += std::fabs(filteredMod);
        }
        float env = envSum / numSamples;

        float smoothedEnv = smoothingFactor * previousEnvelopes[band]
            + (1.0f - smoothingFactor) * env;
        previousEnvelopes[band] = smoothedEnv;

        // zastosuj scaling factor - ciezko dobrac dobra wartosc
        float envelopeGain = smoothedEnv * 200.0f;
        bandEnvelopes[band] = envelopeGain;

        // przetwarzamy carrier przez filtr i stosujemy obwiednie
        const float* carrierLeft = carrierBuffer.getReadPointer(0);
        float* outLeft = outputBuffer.getWritePointer(0);
        const float* carrierRight = (carrierBuffer.getNumChannels() > 1) ? carrierBuffer.getReadPointer(1) : nullptr;
        float* outRight = (outputBuffer.getNumChannels() > 1) ? outputBuffer.getWritePointer(1) : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            float processedCarrierL = carrierFiltersLeft[band].processSample(carrierLeft[i]);
            outLeft[i] += processedCarrierL * envelopeGain;
            if (outRight != nullptr && carrierRight != nullptr)
            {
                float processedCarrierR = carrierFiltersRight[band].processSample(carrierRight[i]);
                outRight[i] += processedCarrierR * envelopeGain;
            }
        }
    }
}
