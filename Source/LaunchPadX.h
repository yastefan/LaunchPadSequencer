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
    Off = 0, White = 3, Red = 5, LightGreen = 24, LightBlue = 40, LightPurple = 48, Orange = 108, LightYellow = 109
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
enum GrandButtons
{
    Flash = 1,
    Swop = 2
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
    void copySteps();
    void deleteSteps();
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
    void changeSequenceOffset();
    void setLed(unsigned char led, Color color, LightMode mode = LightMode::Static);
    void setLeds(unsigned char* leds, unsigned char length, Color color, LightMode mode = LightMode::Static);
    void loadStep(int step);
    void loadPage(int page);
    void checkIpBlock();
    void connectOsc();
    void sendOscMessages();
    void sendOscSequencesMessage(int sequenceNumber, int value, GrandButtons = GrandButtons::Flash);
    int MidiNumberToSequenceNumber(int midiNumber);
    void offAllSequences();
private:
    int sequencerPads[8] = { 81, 82, 83, 84, 85, 86, 87, 88 };
    int sequencerSteps[4] = {4, 4, 4, 4};
    int currentStep = 0;
    int sequenceOffset = 200;

    juce::OSCSender oscSender;
    TapStatus tapStatus;
    StepManager stepManager;
    std::unique_ptr<MidiComponent> midi;

    juce::TextButton sequencerButton{ "Sequencer" };
    juce::TextButton liveButton{ "Live" };
    juce::TextEditor offsetSelector;
    juce::TextEditor ip1Selector;
    juce::TextEditor ip2Selector;
    juce::TextEditor ip3Selector;
    juce::TextEditor ip4Selector;
    juce::TextEditor portSelector;
    juce::TextButton connectButton{ "Connect" };

    void timerCallback() final;
    void handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage&) override;

    void resetTimer();
    void updateBpm();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LaunchPad)
};
