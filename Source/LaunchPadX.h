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
    Off = 0, White = 3, Red = 5, LightGreen = 24, LightPurple = 48, 
};
enum LightMode
{
    Static = 0, Pulse = 2
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
    int tapCount = 0;
};

class StepManager
{
private:

public:
    int activePage = 0;
    int activeStep = 0;
    int statusStorage[4][8][80] = {};    //Page, Step, LED

    void changePage(int page);
    void changeStep(int step);
    int toggleLed(int led, int value = 1);
};

class LaunchPad : public juce::Component,
                  private juce::MidiInputCallback,
                  private juce::MidiMessage,
                  private juce::Timer
{
public:
    LaunchPad();
    ~LaunchPad();

    void paint(juce::Graphics&) override;
    void resized() override;

    void setToProgrammerMode();
    void setToLiveMode();
    void setLed(unsigned char led, Color color, LightMode mode = LightMode::Static);
    void setLeds(unsigned char* leds, unsigned char length, Color color, LightMode mode);
    void loadStep(int step);
    void sendOscMessages();
    void sendOscSequencesMessage(int sequenceNumber, int value);
    int LaunchPad::MidiNumberToSequenceNumber(int midiNumber);
    void offAllSequences();
private:
    int sequencerPads[8] = { 81, 82, 83, 84, 85, 86, 87, 88 };
    int sequencerSteps = 8;
    int currentStep = 0;
    juce::OSCSender oscSender;
    TapStatus tapStatus;
    StepManager stepManager;
    std::unique_ptr<MidiComponent> midi;

    juce::TextButton startButton{ "Start" };

    void timerCallback() final;
    void handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage&) override;

    void resetTimer();
    void updateBpm();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LaunchPad)
};
