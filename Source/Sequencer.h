/*
  ==============================================================================

    Sequencer.h
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
struct LaunchPadCommand : juce::MidiMessage
{
    MidiMessage setProgrammerMode = createSysExMessage("\x00\x20\x29\x02\x0c\x0e\x01", 7);
    MidiMessage setLiveMode = createSysExMessage("\x00\x20\x29\x02\x0c\x0e\x00", 7);
};


class LaunchPad  : public juce::Component,
                          juce::MidiInputCallback
{
public:
    LaunchPad();
    ~LaunchPad();

    void paint(juce::Graphics&) override;
    void resized() override;
private:
    LaunchPadCommand launchPadCommand;
    std::unique_ptr<MidiComponent> midi;

    void handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage&) override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LaunchPad)
};

class Sequencer
{
public:
    Sequencer();
    ~Sequencer();
private:
   
};
