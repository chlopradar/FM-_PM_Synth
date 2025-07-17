/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Resources/Binary/BinaryData.h"
#include "UI/ImageSelector.h"

FM_SYNTHAudioProcessorEditor::FM_SYNTHAudioProcessorEditor(FM_SYNTHAudioProcessor& p)
    : AudioProcessorEditor(&p),
    audioProcessor(p),
    filter(audioProcessor.apvts, "FILTERTYPE", "FILTERFREQ", "FILTERRES"),
    modAdsr("Mod Envelope", audioProcessor.apvts, "MODATTACK", "MODDECAY", "MODSUSTAIN", "MODRELEASE", 0)
{
    // init oscylatorow
    osc1 = std::make_unique<OscComponent>(audioProcessor.apvts, "OSC1WAVETYPE", "OSC1COARSE", "OSC1FINE", "OSC1GAIN", 1);
    osc1->setOscName("Oscillator 1");
    osc2 = std::make_unique<OscComponent>(audioProcessor.apvts, "OSC2WAVETYPE", "OSC2COARSE", "OSC2FINE", "OSC2GAIN", 2);
    osc2->setOscName("Oscillator 2");
    osc3 = std::make_unique<OscComponent>(audioProcessor.apvts, "OSC3WAVETYPE", "OSC3COARSE", "OSC3FINE", "OSC3GAIN", 3);
    osc3->setOscName("Oscillator 3");
    osc4 = std::make_unique<OscComponent>(audioProcessor.apvts, "OSC4WAVETYPE", "OSC4COARSE", "OSC4FINE", "OSC4GAIN", 4);
    osc4->setOscName("Oscillator 4");

    // vocoder
    vocoderToggle.setButtonText("Vocoder");
    vocoderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "VOCODER", vocoderToggle);

    // smoothing slider
    smoothingLabel.setText("Smoothing Factor", juce::dontSendNotification);
    smoothingLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(smoothingLabel);

    smoothingSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    smoothingSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);

    smoothingSlider.setColour(juce::Slider::trackColourId, juce::Colours::lightgrey);
    smoothingSlider.setColour(juce::Slider::thumbColourId, juce::Colours::grey);
    smoothingSlider.setColour(juce::Slider::backgroundColourId, juce::Colours::darkgrey);

    addAndMakeVisible(smoothingSlider);

    smoothingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "SMOOTHFAC", smoothingSlider);

    // adsr
    adsr1 = std::make_unique<AdsrComponent>("Osc 1 Envelope", audioProcessor.apvts, "OSC1ATTACK", "OSC1DECAY", "OSC1SUSTAIN", "OSC1RELEASE", 1);
    adsr2 = std::make_unique<AdsrComponent>("Osc 2 Envelope", audioProcessor.apvts, "OSC2ATTACK", "OSC2DECAY", "OSC2SUSTAIN", "OSC2RELEASE", 2);
    adsr3 = std::make_unique<AdsrComponent>("Osc 3 Envelope", audioProcessor.apvts, "OSC3ATTACK", "OSC3DECAY", "OSC3SUSTAIN", "OSC3RELEASE", 3);
    adsr4 = std::make_unique<AdsrComponent>("Osc 4 Envelope", audioProcessor.apvts, "OSC4ATTACK", "OSC4DECAY", "OSC4SUSTAIN", "OSC4RELEASE", 4);

    // oscilloscope
    oscilloscope = std::make_unique<OscilloscopeComponent>();
    addAndMakeVisible(*oscilloscope);
    startTimerHz(60);

    // obrazki do selektora algorytmow
    std::vector<juce::Image> algorithmImages;
    algorithmImages.push_back(juce::ImageCache::getFromMemory(BinaryData::alg1_png, BinaryData::alg1_pngSize));
    algorithmImages.push_back(juce::ImageCache::getFromMemory(BinaryData::alg2_png, BinaryData::alg2_pngSize));
    algorithmImages.push_back(juce::ImageCache::getFromMemory(BinaryData::alg3_png, BinaryData::alg3_pngSize));
    algorithmImages.push_back(juce::ImageCache::getFromMemory(BinaryData::alg4_png, BinaryData::alg4_pngSize));
    algorithmImages.push_back(juce::ImageCache::getFromMemory(BinaryData::alg5_png, BinaryData::alg5_pngSize));
    algorithmImages.push_back(juce::ImageCache::getFromMemory(BinaryData::alg6_png, BinaryData::alg6_pngSize));
    algorithmImages.push_back(juce::ImageCache::getFromMemory(BinaryData::alg7_png, BinaryData::alg7_pngSize));
    algorithmImages.push_back(juce::ImageCache::getFromMemory(BinaryData::alg8_png, BinaryData::alg8_pngSize));

    // selektor algorytmu
    genericAlgSelector = std::make_unique<GenericImageSelector>(audioProcessor.apvts, "ALGORITHM", algorithmImages, 1111);
    addAndMakeVisible(*genericAlgSelector);

    // do widoku
    addAndMakeVisible(*osc1);
    addAndMakeVisible(*osc2);
    addAndMakeVisible(*osc3);
    addAndMakeVisible(*osc4);
    addAndMakeVisible(*adsr1);
    addAndMakeVisible(*adsr2);
    addAndMakeVisible(*adsr3);
    addAndMakeVisible(*adsr4);
    addAndMakeVisible(filter);
    addAndMakeVisible(modAdsr);
    addAndMakeVisible(vocoderToggle);

    setSize(1100, 1000);
}


FM_SYNTHAudioProcessorEditor::~FM_SYNTHAudioProcessorEditor()
{
}

void FM_SYNTHAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(56, 56, 56));
}

void FM_SYNTHAudioProcessorEditor::resized()
{
    const auto oscWidth = 530;
    const auto oscHeight = 160;
    const auto padding = 10;

    // oscylatorki
    osc1->setBounds(padding, padding, oscWidth, oscHeight);
    osc2->setBounds(padding, osc1->getBottom() + padding, oscWidth, oscHeight);
    osc3->setBounds(padding, osc2->getBottom() + padding, oscWidth, oscHeight);
    osc4->setBounds(padding, osc3->getBottom() + padding, oscWidth, oscHeight);

    // adsr i filtry
    adsr1->setBounds(osc1->getRight() + padding, padding, oscWidth, oscHeight);
    adsr2->setBounds(osc2->getRight() + padding, adsr1->getBottom() + padding, oscWidth, oscHeight);
    adsr3->setBounds(osc3->getRight() + padding, adsr2->getBottom() + padding, oscWidth, oscHeight);
    adsr4->setBounds(osc4->getRight() + padding, adsr3->getBottom() + padding, oscWidth, oscHeight);

    filter.setBounds(padding, osc4->getBottom() + padding, 300, 150);
    modAdsr.setBounds(filter.getRight() + padding, osc4->getBottom() + padding, 400, 150);
    vocoderToggle.setBounds(10, filter.getBottom() + 10, 100, 25);

    smoothingLabel.setBounds(vocoderToggle.getRight() + 10, vocoderToggle.getY(), 200, vocoderToggle.getHeight());
    smoothingSlider.setBounds(smoothingLabel.getRight() - 70, vocoderToggle.getY(), 300, vocoderToggle.getHeight());

    oscilloscope->setBounds(0, vocoderToggle.getBottom() + padding, 1100, 100);

    // selektor algorytmu
    genericAlgSelector->setBounds(modAdsr.getRight() + padding, modAdsr.getBottom() - 125, 350, 125);
}

void FM_SYNTHAudioProcessorEditor::timerCallback()
{
    // pobierz bufor oscyloskopowy z procesora
    juce::AudioBuffer<float> oscBuffer;
    audioProcessor.getOscilloscopeBuffer(oscBuffer);

    // pobierz aktualna czestotliwosc z procesora
    float currentFrequency = audioProcessor.getCurrentFrequency();
    double sampleRate = audioProcessor.getSampleRate();
    // okres = sampleRate / frequency
    int periodSamples = static_cast<int>(sampleRate / currentFrequency);

    // liczba probka ma sie miescic w dostepncyh probkach
    periodSamples = juce::jlimit(1, oscBuffer.getNumSamples(), periodSamples);

    // wektor z probkami
    std::vector<float> samples(periodSamples);
    if (oscBuffer.getNumChannels() > 0)
    {
        const float* data = oscBuffer.getReadPointer(0);

        int startSample = oscBuffer.getNumSamples() - periodSamples;
        std::copy(data + startSample, data + startSample + periodSamples, samples.begin());
    }

    // probki -> oscilloscope
    oscilloscope->pushSamples(samples.data(), periodSamples);
}
