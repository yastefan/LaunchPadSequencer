/*
  ==============================================================================

    Sequencer.cpp
    Created: 14 Jun 2022 12:18:20pm
    Author:  Stefan

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Sequencer.h"

//==============================================================================
Sequencer::Sequencer()
{
}

Sequencer::~Sequencer()
{
}

//==============================================================================

LaunchPad::LaunchPad() :
    midi(new MidiComponent(this))
{
    addAndMakeVisible(*midi);
    midi->sendToOutputs(launchPadCommand.setProgrammerMode);

    setSize(732, 520);
}

LaunchPad::~LaunchPad()
{
    midi->sendToOutputs(launchPadCommand.setLiveMode);
}

void LaunchPad::handleIncomingMidiMessage(juce::MidiInput* /*source*/, const juce::MidiMessage& message)
{
    if (message.isNoteOn()) {
        DBG("NoteNumber: " + std::to_string(message.getNoteNumber()));
    }
    else if (message.isSysEx()) {
        DBG("SysEX:");
    }
    else if (message.isController()) {
        DBG("Control Change: " + std::to_string(message.getControllerNumber()));
    }
}

void LaunchPad::paint(juce::Graphics& g)
{

}

void LaunchPad::resized()
{
    midi->setBounds(getLocalBounds());
}