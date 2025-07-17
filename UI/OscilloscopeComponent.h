// =================== OscilloscopeComponent.h ===================

/*
  ==============================================================================

    OscilloscopeComponent.h
    Created: 30 Mar 2025 10:03:32pm
    Author:  majab

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <vector>

class OscilloscopeComponent : public juce::Component,
    public juce::Timer
{
public:
    OscilloscopeComponent();
    ~OscilloscopeComponent() override;

    void pushSamples(const float* samples, int numSamples);

    void paint(juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;

private:
    std::vector<float> waveform;
    juce::CriticalSection bufferLock;
    std::vector<float> circularBuffer;
    size_t writeIndex = 0;
    const size_t bufferSize = 1024;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopeComponent)
};