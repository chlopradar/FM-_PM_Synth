/*
  ==============================================================================

    FilterComponent.cpp
    Created: 15 Dec 2024 10:12:03pm
    Author:  majab

  ==============================================================================
*/

#include <JuceHeader.h>
#include "FilterComponent.h"

FilterComponent::FilterComponent(juce::AudioProcessorValueTreeState& apvts, juce::String filterTypeSelectorId, 
    juce::String filterFreqId, juce::String filterResId)
{
    juce::StringArray choices{ "Low-pass", "Band-pass", "High-pass" };
    filterTypeSelector.addItemList(choices, 1);
    addAndMakeVisible(filterTypeSelector);

    filterTypeSelectorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts,
        filterTypeSelectorId, filterTypeSelector);

    filterSelectorLabel.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    filterSelectorLabel.setFont(15.0f);
    filterSelectorLabel.setJustificationType(juce::Justification::left);
    addAndMakeVisible(filterSelectorLabel);

    filterOnAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        apvts, "FILTERON", filterOnToggle);
    addAndMakeVisible(filterOnToggle);

    setSliderWithLabel(filterFreqSlider, filterFreqLabel, apvts, filterFreqId, filterFreqAttachment);
    setSliderWithLabel(filterResSlider, filterResLabel, apvts, filterResId, filterResAttachment);
}

FilterComponent::~FilterComponent()
{
}

void FilterComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced(5);
    auto labelSpace = bounds.removeFromTop(25.0f);

    g.fillAll(juce::Colour::fromRGB(56, 56, 56));
    g.setColour(juce::Colours::grey);
    g.setFont(20.0f);
    g.drawText("Filter", labelSpace.withX(5), juce::Justification::left);
    g.drawRoundedRectangle(bounds.toFloat(), 5.0f, 2.0f);
}

void FilterComponent::resized()
{
    const auto startY = 50;
    const auto sliderWidth = 100;
    const auto sliderHeight = 90;
    const auto labelYOffset = 20;
    const auto labelHeight = 20;


    filterTypeSelector.setBounds(10, startY + labelYOffset + 10, 90, 30);
    filterSelectorLabel.setBounds(7, startY +10, 90, labelHeight);
    filterTypeSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(96, 96, 96));

    filterFreqSlider.setBounds(filterTypeSelector.getRight(), startY, sliderWidth, sliderHeight);
    filterFreqLabel.setBounds(filterFreqSlider.getX(), filterFreqSlider.getY() - labelYOffset, filterFreqSlider.getWidth(), labelHeight);

    filterResSlider.setBounds(filterFreqSlider.getRight(), startY, sliderWidth, sliderHeight);
    filterResLabel.setBounds(filterResSlider.getX(), filterResSlider.getY() - labelYOffset, filterResSlider.getWidth(), labelHeight);

    filterOnToggle.setBounds(7, 35, 100, 20);

}

using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

void FilterComponent::setSliderWithLabel(juce::Slider& slider, juce::Label& label, juce::AudioProcessorValueTreeState& apvts, juce::String paramId, std::unique_ptr<Attachment>& attachment)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 75, 25);
    addAndMakeVisible(slider);

    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::lightgrey);
    slider.setColour(juce::Slider::thumbColourId, juce::Colours::grey);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::darkgrey);

    attachment = std::make_unique<Attachment>(apvts, paramId, slider);

    if (paramId == "FILTERFREQ")
    {
        slider.textFromValueFunction = [](double v)
            {
                return juce::String(v, 0) + " Hz";
            };
        slider.updateText();          // musi odswie¿yæ bo inaczej trzeba kliknac
    }

    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(15.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}