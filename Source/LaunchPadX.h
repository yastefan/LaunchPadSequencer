/*
  ==============================================================================

    LaunchPadX.h
    Created: 14 Jun 2022 12:18:20pm
    Author:  Stefan

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MidiComponent.h"

//==============================================================================
/*
*/
enum Color
{
    Off = 0, White = 3, Red = 5
};
enum LightMode
{
    Static = 0, Pulse = 2
};

class LaunchPad : public juce::Component,
                  private juce::MidiInputCallback,
                  private juce::MidiMessage
{
public:
    LaunchPad();
    ~LaunchPad();

    void paint(juce::Graphics&) override;
    void resized() override;

    void setToProgrammerMode();
    void setToLiveMode();
    void setLed(unsigned char led, Color color, LightMode mode = LightMode::Static);
    void setLed(unsigned char* leds, unsigned char length, Color color, LightMode mode);

private:
    int sequencerPads[8] = { 81, 82, 83, 84, 85, 86, 87, 88 };
    int sequencerLeds[8] = { 80, 81, 82, 83, 84, 85, 86, 87 };

    std::unique_ptr<MidiComponent> midi;

    juce::TextButton startButton{ "Start" };

    void handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage&) override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LaunchPad)
};
