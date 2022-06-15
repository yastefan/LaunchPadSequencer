/*
  ==============================================================================

    LaunchPadX.cpp
    Created: 15 Jun 2022 3:29:12pm
    Author:  Stefan

  ==============================================================================
*/

#include "LaunchPadX.h"

LaunchPad::LaunchPad() :
    midi(new MidiComponent(this))
{
    addAndMakeVisible(*midi);
    addAndMakeVisible(startButton);

    startButton.onClick = [this]{ setToProgrammerMode(); };
    setSize(732, 520);
}

LaunchPad::~LaunchPad()
{
    setToLiveMode();
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
//==============================================================================
void LaunchPad::setToProgrammerMode()
{
    midi->sendToOutputs(createSysExMessage("\x00\x20\x29\x02\x0c\x0e\x01", 7));
    //setLed(75, Color::Red);
    unsigned char k[10] = {11, 12, 15, 17, 23, 45, 67, 91, 74, 75};
    setLed(k, 10, Color::White, LightMode::Pulse);
}

void LaunchPad::setToLiveMode()
{
    midi->sendToOutputs(createSysExMessage("\x00\x20\x29\x02\x0c\x0e\x00", 7));
}
void LaunchPad::setLed(unsigned char led, Color color, LightMode mode) 
{
    unsigned char data[9]{ 0x00, 0x20, 0x29, 0x02, 0x0c, 0x03, (unsigned char)mode, (unsigned char)led, (unsigned char)color };
    midi->sendToOutputs(createSysExMessage(data, 9));
}
void LaunchPad::setLed(unsigned char* leds, unsigned char length, Color color, LightMode mode)
{
    int dataSize = 3 * length + 6;
    unsigned char* data = new unsigned char[dataSize] { 0x00, 0x20, 0x29, 0x02, 0x0c, 0x03 };

    for (int i = 6; i < dataSize; i += 3)
    {
        data[i] = (unsigned char)mode;
        data[i + 1] = leds[(i - 6) / 3];
        data[i + 2] = (unsigned char)color;
    }
    midi->sendToOutputs(createSysExMessage(data, dataSize));
    delete[] data;
}
//==============================================================================
void LaunchPad::paint(juce::Graphics& g)
{

}

void LaunchPad::resized()
{
    auto margin = 10;
    midi->setBounds(getLocalBounds());
    startButton.setBounds(margin, (getHeight() / 2),
        getWidth() - (2 * margin), 24);
}