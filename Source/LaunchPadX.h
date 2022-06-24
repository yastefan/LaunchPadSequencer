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
enum TimerNr
{
    Sequencer = 0, Tap = 1
};
enum LaunchKeys
{
    ResyncKey = 17,
    TapKey = 18
};
struct TapStatus
{
    int currentBpmTime = (60 * 1000) / 128;
    long int timeOfLastTap = 0;
    bool active = false;
};

class LaunchPad : public juce::Component,
                  private juce::MidiInputCallback,
                  private juce::MidiMessage,
                  private juce::MultiTimer
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

    void func1();
    void func2();

private:
    int sequencerPads[8] = { 81, 82, 83, 84, 85, 86, 87, 88 };
    int sequencerSteps = 8;
    int currentStep = 0;
    TapStatus tapStatus;
    std::unique_ptr<MidiComponent> midi;

    juce::TextButton startButton{ "Start" };
    juce::TextButton func1Button{ "Funktion 1" };
    juce::TextButton func2Button{ "Funktion 2" };

    void timerCallback(int id) final;
    void handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage&) override;

    void resetTimer();
    void updateBpm();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LaunchPad)
};
