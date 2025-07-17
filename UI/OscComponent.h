/*
  ==============================================================================

    OscComponent.h
    Created: 3 Dec 2024 12:17:04am
    Author:  majab

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../UI/ImageSelector.h"  // GenericImageSelector

class OscComponent : public juce::Component
{
public:
    OscComponent(juce::AudioProcessorValueTreeState& apvts,
        juce::String waveSelectorId, juce::String coarseId, juce::String fineId, juce::String gainId,
        int oscIndex);
    ~OscComponent() override;

    void setOscName(const juce::String& name);

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::String oscillatorName;
    int oscillatorIndex = 1;

    std::unique_ptr<GenericImageSelector> waveSelector;

    juce::Slider coarseSlider, fineSlider, gainSlider;
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> coarseAttachment, fineAttachment, gainAttachment;

    juce::Label waveSelectorLabel{ "Wave Type", "Wave Type" };
    juce::Label coarseLabel{ "Coarse", "Coarse" };
    juce::Label fineLabel{ "Fine", "Fine" };
    juce::Label gainLabel{ "Gain", "Gain" };

    void setSliderWithLabel(juce::Slider& slider, juce::Label& label,
        juce::AudioProcessorValueTreeState& apvts, juce::String paramId,
        std::unique_ptr<Attachment>& attachment);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscComponent)
};