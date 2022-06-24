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
    addAndMakeVisible(func1Button);
    addAndMakeVisible(func2Button);

    startTimer(0, (60 * 1000) / 120);
    startButton.onClick = [this]{ setToProgrammerMode(); };
    func1Button.onClick = [this]{ func1(); };
    func2Button.onClick = [this]{ func2(); };
    setSize(732, 520);
}

LaunchPad::~LaunchPad()
{
    setToLiveMode();
}

void LaunchPad::handleIncomingMidiMessage(juce::MidiInput* /*source*/, const juce::MidiMessage& message)
{
    if (message.isNoteOn()) 
    {
        int noteNumber = message.getNoteNumber();
        switch(noteNumber)
        {
        case LaunchKeys::ResyncKey:
                resetTimer();
                break;
            case LaunchKeys::TapKey:
                updateBpm();
                break;
        }
        DBG("NoteNumber: " + std::to_string(noteNumber));
    }
    else if (message.isSysEx()) {
        DBG("SysEX:");
    }
    else if (message.isController()) {
        DBG("Control Change: " + std::to_string(message.getControllerNumber()) + "  ->  " + std::to_string(message.getControllerValue()));
    }
}

void LaunchPad::timerCallback(int id) {
   
    if (id == TimerNr::Sequencer)
    {
        setLed(sequencerPads[currentStep], Color::Off);
        if (currentStep < sequencerSteps - 1)
            currentStep++;
        else
            currentStep = 0;
        setLed(sequencerPads[currentStep], Color::Red);
    }
    else if (id == TimerNr::Tap)
    {

    }
};

void LaunchPad::resetTimer()
{
    stopTimer(0);
    setLed(sequencerPads[currentStep], Color::Off);
    setLed(sequencerPads[0], Color::Red);
    currentStep = 0;
    startTimer(0, tapStatus.currentBpmTime);
}

void LaunchPad::updateBpm()
{
    long int currentTime = juce::Time::currentTimeMillis();
    long int timeSinceLastTap = currentTime - tapStatus.timeOfLastTap;

    if (timeSinceLastTap < ((60 * 1000) / 50) && timeSinceLastTap > ((60 * 1000) / 240)) // time intervall is between 50 and 240 bpm
    {
        if (tapStatus.active) {
            tapStatus.currentBpmTime = (tapStatus.currentBpmTime + timeSinceLastTap) / 2;
        }
        else 
        {
            tapStatus.currentBpmTime = timeSinceLastTap;
            tapStatus.active = true;
        }
        startTimer(TimerNr::Sequencer, tapStatus.currentBpmTime);
        tapStatus.timeOfLastTap = currentTime;
    }
    else 
    {
        tapStatus.timeOfLastTap = currentTime;
        tapStatus.active = false;
    }
}
//==============================================================================
void LaunchPad::func1()
{
    unsigned char k[10] = {11, 12, 15, 17, 23, 45, 67, 91, 74, 75};
    setLed(k, 10, Color::White, LightMode::Pulse);
}

void LaunchPad::func2()
{
    unsigned char k[10] = { 11, 12, 15, 17, 23, 45, 67, 91, 74, 75 };
    setLed(k, 10, Color::Off, LightMode::Pulse);
}

void LaunchPad::setToProgrammerMode()
{
    midi->sendToOutputs(createSysExMessage("\x00\x20\x29\x02\x0c\x0e\x01", 7));
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
    startButton.setBounds(margin, (getHeight() / 2), getWidth() - (2 * margin), 24);
    func1Button.setBounds(margin, (getHeight() - 100), getWidth() - (2 * margin), 24);
    func2Button.setBounds(margin, (getHeight() - 60), getWidth() - (2 * margin), 24);
}