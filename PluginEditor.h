/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/AdsrComponent.h"
#include "UI/OscComponent.h"
#include "UI/FilterComponent.h"
#include "UI/OscilloscopeComponent.h"
#include "UI/ImageSelector.h"  

class FM_SYNTHAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    FM_SYNTHAudioProcessorEditor(FM_SYNTHAudioProcessor&);
    ~FM_SYNTHAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    FM_SYNTHAudioProcessor& audioProcessor;
    std::unique_ptr<OscComponent> osc1, osc2, osc3, osc4;
    std::unique_ptr<AdsrComponent> adsr1, adsr2, adsr3, adsr4;
    FilterComponent filter;
    AdsrComponent modAdsr;

    juce::ToggleButton vocoderToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> vocoderAttachment;

    juce::Slider smoothingSlider;
    juce::Label smoothingLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> smoothingAttachment;

    std::unique_ptr<OscilloscopeComponent> oscilloscope;

    std::unique_ptr<GenericImageSelector> genericAlgSelector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FM_SYNTHAudioProcessorEditor)
};
