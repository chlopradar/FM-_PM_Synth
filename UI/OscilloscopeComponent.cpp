/*
  ==============================================================================

    OscilloscopeComponent.cpp
    Created: 30 Mar 2025 10:03:32pm
    Author:  majab

  ==============================================================================
*/

#include "OscilloscopeComponent.h"

OscilloscopeComponent::OscilloscopeComponent()
{
    startTimerHz(30); // 30 razy na sek
}

OscilloscopeComponent::~OscilloscopeComponent()
{
    stopTimer();
}

void OscilloscopeComponent::pushSamples(const float* samples, int numSamples)
{
    const juce::ScopedLock sl(bufferLock);
    // zastap bufor nowymi probkami
    circularBuffer.assign(samples, samples + numSamples);
    // reset index
    writeIndex = 0;
}



void OscilloscopeComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(56, 56, 56));
    g.setColour(juce::Colours::lightgrey);

    const juce::ScopedLock sl(bufferLock);
    if (circularBuffer.empty())
        return;

    auto area = getLocalBounds().toFloat();
    juce::Path path;
    int numSamples = static_cast<int>(circularBuffer.size());
    float startX = area.getX();
    float endX = area.getRight();
    float xIncrement = (endX - startX) / (numSamples - 1);

    path.startNewSubPath(startX, area.getCentreY() - circularBuffer[0] * area.getHeight() * 0.5f);
    for (int i = 1; i < numSamples; ++i)
    {
        float x = startX + i * xIncrement;
        float y = area.getCentreY() - circularBuffer[i] * area.getHeight() * 0.5f;
        path.lineTo(x, y);
    }
    g.strokePath(path, juce::PathStrokeType(2.0f));
}




void OscilloscopeComponent::resized()
{
}

void OscilloscopeComponent::timerCallback()
{
    repaint();
}
