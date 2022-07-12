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

void StepManager::changePage(int page) 
{
    if (page >= 0 && page < 4) 
        activePage = page;
}
void StepManager::changeStep(int step)
{
    if (step >= 0 && step < 8)
        activeStep = step;
}
void StepManager::toggleLed(int led, int value)
{
    if (led < 80)
    {
        if (statusStorage[activePage][activeStep][led] > 0)
            statusStorage[activePage][activeStep][led] = 0;
        else
            statusStorage[activePage][activeStep][led] = value;
    }
}
 
void LaunchPad::handleIncomingMidiMessage(juce::MidiInput* /*source*/, const juce::MidiMessage& message)
{
    if (message.isNoteOn()) 
    { 
        int noteNumber = message.getNoteNumber();

        if (noteNumber == LaunchKeys::ResyncKey)
            resetTimer();
        else if (noteNumber == LaunchKeys::TapKey)
            updateBpm();
        else if (noteNumber > 80 && noteNumber < 89)
        {
            DBG("Step: NoteNumber: " + std::to_string(noteNumber));
            setLed(stepManager.activeStep + 81, Color::Off);
            setLed(noteNumber, Color::White);
            stepManager.changeStep(noteNumber - 81);
            loadStep(noteNumber - 81);
        }
        else if (noteNumber > 30 && noteNumber < 79)
        {
            DBG("Exec: NoteNumber: " + std::to_string(noteNumber));
            stepManager.toggleLed(noteNumber);
            setLed(noteNumber, Color::Red);
        }
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
        if (currentStep != stepManager.activeStep)
            setLed(sequencerPads[currentStep], Color::Off);
        else
            setLed(sequencerPads[currentStep], Color::White);
        
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

    if (timeSinceLastTap < ((60 * 1000) / 50) && timeSinceLastTap > ((60 * 1000) / 280)) // time intervall is between 50 and 280 bpm
    {
        if (tapStatus.tapCount) {
            tapStatus.currentBpmTime = (tapStatus.currentBpmTime * tapStatus.tapCount + timeSinceLastTap) / (tapStatus.tapCount + 1);
        }
        else 
        {
            tapStatus.currentBpmTime = timeSinceLastTap;
            tapStatus.tapCount = 1;
        }
        startTimer(TimerNr::Sequencer, tapStatus.currentBpmTime);
        tapStatus.timeOfLastTap = currentTime;
        DBG(60000 / tapStatus.currentBpmTime);
    }
    else 
    {
        tapStatus.timeOfLastTap = currentTime;
        tapStatus.tapCount = 0;
    }
}
//==============================================================================
void LaunchPad::func1()
{
    stepManager.statusStorage[stepManager.activePage][1][15] = 1;
    stepManager.statusStorage[stepManager.activePage][1][14] = 1;
    stepManager.statusStorage[stepManager.activePage][1][13] = 1;
}

void LaunchPad::func2()
{
    unsigned char k[10] = { 11, 12, 15, 17, 23, 45, 67, 91, 74, 75 };
    setLeds(k, 10, Color::Off, LightMode::Pulse);
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
void LaunchPad::loadStep(int step)
{
    unsigned char ledsOn[80] = {};
    unsigned char ledsOff[80] = {};
    int ledsOnCount = 0;
    int ledsOffCount = 0;

    for (int i = 0; i < 80; i++)
    {
        if (stepManager.statusStorage[stepManager.activePage][step][i] > 0)
        {
            ledsOn[ledsOnCount] = i;
            ledsOnCount++;
        }
        else
        {
            ledsOff[ledsOffCount] = i;
            ledsOffCount++;
        }
    }
    setLeds(ledsOn, ledsOnCount, Color::Red, LightMode::Static);
    setLeds(ledsOff, ledsOffCount, Color::Off, LightMode::Static);
}

void LaunchPad::setLeds(unsigned char* leds, unsigned char length, Color color, LightMode mode)
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