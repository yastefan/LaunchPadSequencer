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

    oscSender.connect("127.0.0.1", 9005);

    startTimer((60 * 1000) / 120);
    startButton.onClick = [this]{ setToProgrammerMode(); };
    setSize(732, 520);
}

LaunchPad::~LaunchPad()
{
    oscSender.disconnect();
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
int StepManager::toggleLed(int led, int value)
{
    if (led < 80)
    {
        if (statusStorage[activePage][activeStep][led] > 0)
            statusStorage[activePage][activeStep][led] = 0;
        else
            statusStorage[activePage][activeStep][led] = value;
        return statusStorage[activePage][activeStep][led];
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
            if ( stepManager.toggleLed(noteNumber) )
                setLed(noteNumber, Color::Red);
            else
                setLed(noteNumber, Color::Off);
        }
    }

    else if (message.isController()) {
        DBG("Control Change: " + std::to_string(message.getControllerNumber()) + "  ->  " + std::to_string(message.getControllerValue()));
        if (message.getControllerValue() > 0)
        {
            switch (message.getControllerNumber())
            {
            case 89:
                stepManager.changePage(0);
                offAllSequences();
                setLed(89, Color::Red);
                setLed(79, Color::Off);
                setLed(69, Color::Off);
                setLed(59, Color::Off);
                break;
            case 79:
                stepManager.changePage(1);
                offAllSequences();
                setLed(89, Color::Off);
                setLed(79, Color::Red);
                setLed(69, Color::Off);
                setLed(59, Color::Off);
                break;
            case 69:
                stepManager.changePage(2);
                offAllSequences();
                setLed(89, Color::Off);
                setLed(79, Color::Off);
                setLed(69, Color::Red);
                setLed(59, Color::Off);
                break;
            case 59:
                stepManager.changePage(3);
                offAllSequences();
                setLed(89, Color::Off);
                setLed(79, Color::Off);
                setLed(69, Color::Off);
                setLed(59, Color::Red);
                break;
            default:
                break;
            }
            loadStep(stepManager.activeStep);
        }
    }
}

void LaunchPad::sendOscMessages() {
    
    int precursor = (currentStep > 0) ? currentStep - 1 : 7;

    for (int i = 0; i < 80; i++)
    {
        int sequenceNumber = MidiNumberToSequenceNumber(i) + 208;
        if (stepManager.statusStorage[stepManager.activePage][currentStep][i] > stepManager.statusStorage[stepManager.activePage][precursor][i])
        {
            sendOscSequencesMessage(sequenceNumber, 1);
        }
        else if (stepManager.statusStorage[stepManager.activePage][currentStep][i] < stepManager.statusStorage[stepManager.activePage][precursor][i])
        {
            sendOscSequencesMessage(sequenceNumber, 0);
        }
    }
}

void LaunchPad::offAllSequences()
{
    for (int i = 0; i < 80; i++)
    {
        sendOscSequencesMessage(MidiNumberToSequenceNumber(i)+208, 0);
    }
}


void LaunchPad::sendOscSequencesMessage(int sequenceNumber, int value) {
    std::string message_address = "/13.13.1.5." + std::to_string(sequenceNumber);
    juce::OSCArgument button("Flash");
    oscSender.send(message_address.c_str(), button, value);
}

int LaunchPad::MidiNumberToSequenceNumber(int midiNumber) {
    if (midiNumber < 79 && midiNumber > 70)
        return midiNumber - 71;
    else if (midiNumber < 69 && midiNumber > 60)
        return midiNumber - 53;
    else if (midiNumber < 59 && midiNumber > 50)
        return midiNumber - 35;
    else if (midiNumber < 49 && midiNumber > 40)
        return midiNumber - 17;
    else if (midiNumber < 39 && midiNumber > 30)
        return midiNumber + 1;
    else if (midiNumber < 29 && midiNumber > 20)
        return midiNumber + 19;
    else if (midiNumber < 19 && midiNumber > 10)
        return midiNumber + 37;
}

void LaunchPad::timerCallback() {
   
    if (currentStep != stepManager.activeStep)
        if(sequencerPads[currentStep] < 85)
            setLed(sequencerPads[currentStep], Color::LightGreen);
        else
            setLed(sequencerPads[currentStep], Color::LightPurple);
    else
        setLed(sequencerPads[currentStep], Color::White);
    
    if (currentStep < sequencerSteps - 1)
        currentStep++;
    else
        currentStep = 0;
    setLed(sequencerPads[currentStep], Color::Red);
    oscSender.send("/sequencer/Beat", currentStep);
    sendOscMessages();
};

void LaunchPad::resetTimer()
{
    stopTimer();
    setLed(sequencerPads[currentStep], Color::Off);
    setLed(sequencerPads[0], Color::Red);
    currentStep = 0;
    startTimer(tapStatus.currentBpmTime);
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
        startTimer(tapStatus.currentBpmTime);
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

void LaunchPad::setToProgrammerMode()
{
    midi->sendToOutputs(createSysExMessage("\x00\x20\x29\x02\x0c\x0e\x01", 7));
    setLed(89, Color::Red);
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
    int index = 0;

    for (int row = 3; row < 8; row++) {
        for (int column = 1; column < 9; column++) {
            index = row * 10 + column;
            if (stepManager.statusStorage[stepManager.activePage][step][index] > 0)
            {
                ledsOn[ledsOnCount] = index;
                ledsOnCount++;
            }
            else
            {
                ledsOff[ledsOffCount] = index;
                ledsOffCount++;
            }
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
}