/*
  ==============================================================================

    OscComponent.cpp
    Created: 3 Dec 2024 12:17:04am
    Author:  majab

  ==============================================================================
*/

#include <JuceHeader.h>
#include "OscComponent.h"
#include "../Resources/Binary/BinaryData.h"

OscComponent::OscComponent(juce::AudioProcessorValueTreeState& apvts,
    juce::String waveSelectorId, juce::String coarseId, juce::String fineId, juce::String gainId,
    int oscIndex)
    : oscillatorIndex(oscIndex)
{
    // przygotowujemy obrazki fali
    std::vector<juce::Image> waveImages;
    waveImages.push_back(juce::ImageCache::getFromMemory(BinaryData::sine_png, BinaryData::sine_pngSize));
    waveImages.push_back(juce::ImageCache::getFromMemory(BinaryData::saw_png, BinaryData::saw_pngSize));
    waveImages.push_back(juce::ImageCache::getFromMemory(BinaryData::square_png, BinaryData::square_pngSize));
    waveImages.push_back(juce::ImageCache::getFromMemory(BinaryData::triangle_png, BinaryData::triangle_pngSize));

    // kolor w zal. od osc'a
    juce::Colour waveToggledColour;
    switch (oscIndex)
    {
    case 1: waveToggledColour = juce::Colour::fromRGB(230, 175, 25); break;
    case 2: waveToggledColour = juce::Colour::fromRGB(219, 36, 36);  break;
    case 3: waveToggledColour = juce::Colour::fromRGB(38, 217, 49);  break;
    case 4: waveToggledColour = juce::Colour::fromRGB(38, 97, 217);  break;
    default: waveToggledColour = juce::Colours::white; break;
    }

    // wave selector
    waveSelector = std::make_unique<GenericImageSelector>(
        apvts,
        waveSelectorId,
        waveImages,
        2222,
        waveToggledColour);

    addAndMakeVisible(*waveSelector);

    // init suwakow
    setSliderWithLabel(coarseSlider, coarseLabel, apvts, coarseId, coarseAttachment);
    setSliderWithLabel(fineSlider, fineLabel, apvts, fineId, fineAttachment);
    setSliderWithLabel(gainSlider, gainLabel, apvts, gainId, gainAttachment);
}

OscComponent::~OscComponent()
{
}

void OscComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced(5);
    auto labelSpace = bounds.removeFromTop(25.0f);

    g.fillAll(juce::Colour::fromRGB(56, 56, 56));

    // kolor ramki w zal. od osc'a
    juce::Colour borderColour;
    switch (oscillatorIndex)
    {
    case 1:
        borderColour = juce::Colour::fromRGB(230, 175, 25);
        break;
    case 2:
        borderColour = juce::Colour::fromRGB(219, 36, 36);
        break;
    case 3:
        borderColour = juce::Colour::fromRGB(38, 217, 49);
        break;
    case 4:
        borderColour = juce::Colour::fromRGB(38, 97, 217);
        break;
    default:
        borderColour = juce::Colours::white;
        break;
    }

    g.setColour(borderColour);
    g.drawRoundedRectangle(bounds.toFloat(), 5.0f, 2.0f);

    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText(oscillatorName, labelSpace.withX(5), juce::Justification::left);
}

void OscComponent::resized()
{
    const auto startY = 55;
    const auto sliderWidth = 80;
    const auto sliderHeight = 90;
    const auto labelYOffset = 20;
    const auto labelHeight = 20;

    waveSelector->setBounds(10, 35, 150,115);

    coarseSlider.setBounds(waveSelector->getRight() + 40, startY, sliderWidth, sliderHeight);
    coarseLabel.setBounds(coarseSlider.getX(), coarseSlider.getY() - labelYOffset, sliderWidth, labelHeight);

    fineSlider.setBounds(coarseSlider.getRight() + 16, startY, sliderWidth, sliderHeight);
    fineLabel.setBounds(fineSlider.getX(), fineSlider.getY() - labelYOffset, sliderWidth, labelHeight);

    gainSlider.setBounds(fineSlider.getRight() + 16, startY, sliderWidth, sliderHeight);
    gainLabel.setBounds(gainSlider.getX(), gainSlider.getY() - labelYOffset, sliderWidth, labelHeight);
}

void OscComponent::setSliderWithLabel(juce::Slider& slider, juce::Label& label,
    juce::AudioProcessorValueTreeState& apvts, juce::String paramId,
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& attachment)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
    addAndMakeVisible(slider);

    juce::Colour fillColour, thumbColour, outlineColour;
    switch (oscillatorIndex)
    {
    case 1:
        fillColour = juce::Colour::fromRGB(240, 207, 117);
        thumbColour = juce::Colour::fromRGB(230, 175, 25);
        outlineColour = juce::Colour::fromRGB(138, 105, 15);
        break;
    case 2:
        fillColour = juce::Colour::fromRGB(233, 124, 124);
        thumbColour = juce::Colour::fromRGB(219, 36, 36);
        outlineColour = juce::Colour::fromRGB(131, 22, 22);
        break;
    case 3:
        fillColour = juce::Colour::fromRGB(125, 232, 131);
        thumbColour = juce::Colour::fromRGB(38, 217, 49);
        outlineColour = juce::Colour::fromRGB(23, 130, 29);
        break;
    case 4:
        fillColour = juce::Colour::fromRGB(125, 160, 232);
        thumbColour = juce::Colour::fromRGB(38, 97, 217);
        outlineColour = juce::Colour::fromRGB(23, 58, 130);
        break;
    default:
        fillColour = juce::Colours::grey;
        thumbColour = juce::Colours::grey;
        outlineColour = juce::Colours::lightgrey;
        break;
    }
    slider.setColour(juce::Slider::rotarySliderFillColourId, fillColour);
    slider.setColour(juce::Slider::thumbColourId, thumbColour);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, outlineColour);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramId, slider);

    label.setColour(juce::Label::ColourIds::textColourId, juce::Colours::white);
    label.setFont(15.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void OscComponent::setOscName(const juce::String& name)
{
    oscillatorName = name;
    repaint();
}