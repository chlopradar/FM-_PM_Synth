/*
  ==============================================================================

    ImageSelector.h
    Created: 8 Apr 2025 6:12:08pm
    Author:  majab

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class GenericImageSelector : public juce::Component,
    public juce::Button::Listener
{
public:
    GenericImageSelector(juce::AudioProcessorValueTreeState& vts,
        const juce::String& parameterIDToUse,
        const std::vector<juce::Image>& imagesToUse,
        int groupID,
        juce::Colour toggledColour = juce::Colours::orange)
        : apvts(vts),
        parameterID(parameterIDToUse),
        images(imagesToUse),
        colourWhenToggled(toggledColour)
    {
        // tyle przyciskow ile obrazkow w wektorze
        for (size_t i = 0; i < images.size(); ++i)
        {
            auto* btn = new ImageToggleButton("Btn " + juce::String((int)i + 1),
                images[i],
                colourWhenToggled);
            btn->setClickingTogglesState(true);
            btn->setRadioGroupId(groupID);
            btn->addListener(this);
            buttons.add(btn);
            addAndMakeVisible(btn);
        }
        updateToggleStatesFromParam();
    }

    ~GenericImageSelector() override
    {
        for (auto* b : buttons)
            b->removeListener(this);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour::fromRGB(56, 56, 56));
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(4);
        int numButtons = buttons.size();
        if (numButtons == 0)
            return;

        const int spacing = 6; // odstep między przyciskami

        if (numButtons <= 2)
        {
            int buttonWidth = (bounds.getWidth() - spacing * (numButtons - 1)) / numButtons;
            int buttonHeight = bounds.getHeight();
            int totalWidth = numButtons * buttonWidth + (numButtons - 1) * spacing;
            int startX = bounds.getX() + (bounds.getWidth() - totalWidth) / 2;
            for (int i = 0; i < numButtons; ++i)
            {
                int x = startX + i * (buttonWidth + spacing);
                buttons[i]->setBounds(x, bounds.getY(), buttonWidth, buttonHeight);
            }
        }
        else
        {
            int firstRowCount = (numButtons + 1) / 2; // round w góre
            int secondRowCount = numButtons - firstRowCount;
            int rowHeight = (bounds.getHeight() - spacing) / 2;
            auto topRow = bounds.removeFromTop(rowHeight);
            bounds.removeFromTop(spacing);
            auto bottomRow = bounds;

            int topButtonWidth = (topRow.getWidth() - spacing * (firstRowCount - 1)) / firstRowCount;
            int topUsedWidth = firstRowCount * topButtonWidth + (firstRowCount - 1) * spacing;
            int topStartX = topRow.getX() + (topRow.getWidth() - topUsedWidth) / 2;
            for (int i = 0; i < firstRowCount; ++i)
            {
                int x = topStartX + i * (topButtonWidth + spacing);
                buttons[i]->setBounds(x, topRow.getY(), topButtonWidth, rowHeight);
            }

            int bottomButtonWidth = (bottomRow.getWidth() - spacing * (secondRowCount - 1)) / secondRowCount;
            int bottomUsedWidth = secondRowCount * bottomButtonWidth + (secondRowCount - 1) * spacing;
            int bottomStartX = bottomRow.getX() + (bottomRow.getWidth() - bottomUsedWidth) / 2;
            for (int j = 0; j < secondRowCount; ++j)
            {
                int idx = firstRowCount + j;
                int x = bottomStartX + j * (bottomButtonWidth + spacing);
                buttons[idx]->setBounds(x, bottomRow.getY(), bottomButtonWidth, rowHeight);
            }
        }
    }

    void buttonClicked(juce::Button* b) override
    {
        for (int i = 0; i < buttons.size(); ++i)
        {
            if (b == buttons[i])
            {
                if (auto* param = apvts.getParameter(parameterID))
                {
                    param->beginChangeGesture();
                    float newValue = (buttons.size() > 1) ? (float)i / (buttons.size() - 1) : 0.0f;
                    param->setValueNotifyingHost(newValue);
                    param->endChangeGesture();
                }
                break;
            }
        }
    }

    void updateToggleStatesFromParam()
    {
        if (auto* param = apvts.getParameter(parameterID))
        {
            float value = param->getValue(); // [0..1]
            int index = juce::roundToInt(value * (buttons.size() - 1));
            for (int i = 0; i < buttons.size(); ++i)
                buttons[i]->setToggleState(i == index, juce::dontSendNotification);
        }
    }

private:
    // przycisk o kolorze zaznaczonej ramki
    class ImageToggleButton : public juce::Button
    {
    public:
        ImageToggleButton(const juce::String& name, const juce::Image& img, juce::Colour toggledCol)
            : juce::Button(name), image(img), toggledColour(toggledCol)
        {
            setClickingTogglesState(true);
        }

        void paintButton(juce::Graphics& g,
            bool /*isMouseOverButton*/,
            bool /*isButtonDown*/) override
        {
            g.fillAll(juce::Colour::fromRGB(56, 56, 56)); 

            auto bounds = getLocalBounds().reduced(3).toFloat();
            if (image.isValid())
            {
                g.drawImageWithin(image,
                    (int)bounds.getX(), (int)bounds.getY(),
                    (int)bounds.getWidth(), (int)bounds.getHeight(),
                    juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
            }

            // kolor oscylator albo defaultowy
            juce::Colour borderCol = getToggleState() ? toggledColour : juce::Colours::grey;
            g.setColour(borderCol);
            g.drawRoundedRectangle(getLocalBounds().toFloat(), 5.0f, 2);
        }

    private:
        juce::Image image;
        juce::Colour toggledColour;
    };

    juce::AudioProcessorValueTreeState& apvts;
    juce::String parameterID;
    std::vector<juce::Image> images;
    juce::OwnedArray<ImageToggleButton> buttons;

    juce::Colour colourWhenToggled;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenericImageSelector)
};
