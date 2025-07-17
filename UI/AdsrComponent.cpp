/*
  ==============================================================================

    AdsrComponent.cpp
    Created: 2 Dec 2024 7:38:18pm
    Author:  majab

  ==============================================================================
*/

#include <JuceHeader.h>
#include "AdsrComponent.h"

AdsrComponent::AdsrComponent(juce::String name, juce::AudioProcessorValueTreeState& apvts,
    juce::String attackId, juce::String decayId, juce::String sustainId, juce::String releaseId,
    int oscIndex)
{
    componentName = name;
    oscillatorIndex = oscIndex;

    setSliderWithLabel(attackSlider, attackLabel, apvts, attackId, attackAttachment);
    setSliderWithLabel(decaySlider, decayLabel, apvts, decayId, decayAttachment);
    setSliderWithLabel(sustainSlider, sustainLabel, apvts, sustainId, sustainAttachment);
    setSliderWithLabel(releaseSlider, releaseLabel, apvts, releaseId, releaseAttachment);
}

AdsrComponent::~AdsrComponent()
{
}

void AdsrComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced(5);
    auto labelSpace = bounds.removeFromTop(25.0f);

    g.fillAll(juce::Colour::fromRGB(56, 56, 56));

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
        borderColour = juce::Colours::grey;
        break;
    }

    g.setColour(borderColour);
    g.drawRoundedRectangle(bounds.toFloat(), 5.0f, 2.0f);

    g.setColour(juce::Colour::fromRGB(146, 146, 146));
    g.setFont(20.0f);
    g.drawText(componentName, labelSpace.withX(5), juce::Justification::left);
}


void AdsrComponent::resized()
{
    const auto bounds = getLocalBounds().reduced(10);
    const auto padding = 10;
    const auto sliderWidth = bounds.getWidth() / 4 - padding;
    const auto sliderHeight = bounds.getHeight() - 45;
    const auto sliderStartX = padding + 5;
    const auto sliderStartY = 55;
    const auto labelYOffset = 20;
    const auto labelStart = sliderStartY - labelYOffset;

    attackSlider.setBounds(sliderStartX, sliderStartY, sliderWidth, sliderHeight);
    attackLabel.setBounds(attackSlider.getX(), labelStart, sliderWidth, labelYOffset);

    decaySlider.setBounds(attackSlider.getRight() + padding, sliderStartY, sliderWidth, sliderHeight);
    decayLabel.setBounds(decaySlider.getX(), labelStart, sliderWidth, labelYOffset);

    sustainSlider.setBounds(decaySlider.getRight() + padding, sliderStartY, sliderWidth, sliderHeight);
    sustainLabel.setBounds(sustainSlider.getX(), labelStart, sliderWidth, labelYOffset);

    releaseSlider.setBounds(sustainSlider.getRight() + padding, sliderStartY, sliderWidth, sliderHeight);
    releaseLabel.setBounds(releaseSlider.getX(), labelStart, sliderWidth, labelYOffset);
}

using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
void AdsrComponent::setSliderWithLabel(juce::Slider& slider,
    juce::Label& label,
    juce::AudioProcessorValueTreeState& apvts,
    juce::String                                paramId,
    std::unique_ptr<Attachment>& attachment)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
    addAndMakeVisible(slider);

    juce::Colour trackColour, thumbColour, backgroundColour;
    switch (oscillatorIndex)
    {
    case 1: trackColour = juce::Colour::fromRGB(240, 207, 117);
        thumbColour = juce::Colour::fromRGB(230, 175, 25);
        backgroundColour = juce::Colour::fromRGB(138, 105, 15); break;
    case 2: trackColour = juce::Colour::fromRGB(233, 124, 124);
        thumbColour = juce::Colour::fromRGB(219, 36, 36);
        backgroundColour = juce::Colour::fromRGB(131, 22, 22);  break;
    case 3: trackColour = juce::Colour::fromRGB(125, 232, 131);
        thumbColour = juce::Colour::fromRGB(38, 217, 49);
        backgroundColour = juce::Colour::fromRGB(23, 130, 29);  break;
    case 4: trackColour = juce::Colour::fromRGB(125, 160, 232);
        thumbColour = juce::Colour::fromRGB(38, 97, 217);
        backgroundColour = juce::Colour::fromRGB(23, 58, 130);  break;
    default: trackColour = juce::Colours::lightgrey;
        thumbColour = juce::Colours::grey;
        backgroundColour = juce::Colours::darkgrey;           break;
    }
    slider.setColour(juce::Slider::trackColourId, trackColour);
    slider.setColour(juce::Slider::thumbColourId, thumbColour);
    slider.setColour(juce::Slider::backgroundColourId, backgroundColour);

    attachment = std::make_unique<Attachment>(apvts, paramId, slider);

    // jednostki i symbole
    if (paramId.contains("ATTACK") || paramId.contains("DECAY") || paramId.contains("RELEASE"))
    {
        slider.setTextValueSuffix(" ms");
        slider.setNumDecimalPlacesToDisplay(1);
    }
    else if (paramId.contains("SUSTAIN") && paramId.startsWith("OSC"))
    {
        const auto infinity = juce::String("-") + juce::String::fromUTF8("\xE2\x88\x9E", 3);  //

        slider.textFromValueFunction = [infinity](double v)
            {
                return (v <= -60.0) ? infinity + " dB"
                    : juce::String(v, 1) + " dB";
            };

        slider.valueFromTextFunction = [infinity](const juce::String& t)
            {
                if (t.trim().startsWith(infinity))
                    return -60.0;
                return t.getDoubleValue();
            };
        slider.updateText();
    }
    else if (paramId == "MODATTACK" || paramId == "MODDECAY" ||
        paramId == "MODRELEASE")
    {
        slider.setTextValueSuffix(" ms");
        slider.setNumDecimalPlacesToDisplay(1);
    }
    else if (paramId == "MODSUSTAIN")
    {
        slider.setTextValueSuffix(" %");
        slider.setNumDecimalPlacesToDisplay(1);
    }

    label.setColour(juce::Label::textColourId, juce::Colours::white);
    label.setFont(15.0f);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}
