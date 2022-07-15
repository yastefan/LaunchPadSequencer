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
    addAndMakeVisible(liveButton);
    addAndMakeVisible(offsetSelector);

    addAndMakeVisible(ip1Selector);
    addAndMakeVisible(ip2Selector);
    addAndMakeVisible(ip3Selector);
    addAndMakeVisible(ip4Selector);
    addAndMakeVisible(portSelector);
    addAndMakeVisible(connectButton);

    offsetSelector.setInputRestrictions(4, "0123456789");
    offsetSelector.setText("200");
    ip1Selector.setInputRestrictions(3, "0123456789");
    ip1Selector.setText("127");
    ip1Selector.onTextChange = [this] { checkIpBlock(); };
    ip2Selector.setInputRestrictions(3, "0123456789");
    ip2Selector.setText("0");
    ip2Selector.onTextChange = [this] { checkIpBlock(); };
    ip3Selector.setInputRestrictions(3, "0123456789");
    ip3Selector.setText("0");
    ip3Selector.onTextChange = [this] { checkIpBlock(); };
    ip4Selector.setInputRestrictions(3, "0123456789");
    ip4Selector.setText("1");
    ip4Selector.onTextChange = [this] { checkIpBlock(); };
    portSelector.setInputRestrictions(5, "0123456789");
    portSelector.setText("9005");

    offsetSelector.onTextChange = [this] {changeSequenceOffset(); };
    startButton.onClick = [this] { setToProgrammerMode(); };
    liveButton.onClick = [this] { setToLiveMode(); };
    connectButton.onClick = [this] { connectOsc(); };

    oscSender.connect("127.0.0.1", 9005);
    startTimer((60 * 1000) / 120);
    
    setSize(500, 500);
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
void StepManager::copySteps()
{
    for (int step = 0; step < 4; step++) {
        for (int row = 5; row < 8; row++) {
            for (int column = 1; column < 9; column++) {
                statusStorage[activePage][step + 4][row * 10 + column] = statusStorage[activePage][step][row * 10 + column];
            }
        }
    }
}
void StepManager::deleteSteps()
{
    for (int step = 0; step < 8; step++) {
        for (int row = 5; row < 8; row++) {
            for (int column = 1; column < 9; column++) {
                statusStorage[activePage][step][row * 10 + column] = 0;
            }
        }
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
            if (stepManager.activeStep < 4)
                setLed(stepManager.activeStep + 81, Color::LightBlue);
            else
                setLed(stepManager.activeStep + 81, Color::LightPurple);
            setLed(noteNumber, Color::White);
            stepManager.changeStep(noteNumber - 81);
            loadStep(noteNumber - 81);
        }
        else if (noteNumber > 50 && noteNumber < 79)
        {
            DBG("Exec: NoteNumber: " + std::to_string(noteNumber));
            if ( stepManager.toggleLed(noteNumber) )
                setLed(noteNumber, Color::Red);
            else
                setLed(noteNumber, Color::LightGreen);
        }
        else if ((noteNumber > 24 && noteNumber < 29) || (noteNumber > 34 && noteNumber < 39) || (noteNumber > 44 && noteNumber < 49))
        {
            if (stepManager.toggleLed(noteNumber))
            {
                setLed(noteNumber, Color::White);
                sendOscSequencesMessage(MidiNumberToSequenceNumber(noteNumber), 1);
            }
            else
            {
                setLed(noteNumber, Color::LightYellow);
                sendOscSequencesMessage(MidiNumberToSequenceNumber(noteNumber), 0);
            }
        }
        else if ((noteNumber > 20 && noteNumber < 25) || (noteNumber > 30 && noteNumber < 35) || (noteNumber > 40 && noteNumber < 45))
        {
            setLed(noteNumber, Color::White);
            sendOscSequencesMessage(MidiNumberToSequenceNumber(noteNumber), 1);
        }
        else if (noteNumber == 12) {
            if (stepManager.toggleLed(noteNumber))
            {
                setLed(noteNumber, Color::Red, LightMode::Pulse);
                stopTimer();
                offAllSequences();
            }
            else
            {
                setLed(noteNumber, Color::Red);
                startTimer(tapStatus.currentBpmTime);
            }
        }
        else if (noteNumber == 11) 
        {
            setToLiveMode();
        }
    }
    else if (message.isNoteOff())
    {
        int noteNumber = message.getNoteNumber();
        if ((noteNumber > 20 && noteNumber < 25) || (noteNumber > 30 && noteNumber < 35) || (noteNumber > 40 && noteNumber < 45))
        {
            setLed(noteNumber, Color::Orange);
            sendOscSequencesMessage(MidiNumberToSequenceNumber(noteNumber), 0);
        }
    }

    else if (message.isController()) {
        DBG("Control Change: " + std::to_string(message.getControllerNumber()) + "  ->  " + std::to_string(message.getChannel()) + " " + std::to_string(message.getControllerValue()));
        if (message.getControllerValue() > 0)
        {
            switch (message.getControllerNumber())
            {
            case 89:
                loadPage(0);
                break;
            case 79:
                loadPage(1);
                break;
            case 69:
                loadPage(2);
                break;
            case 59:
                loadPage(3);
                break;
            case 94:
                stepManager.copySteps();
                loadPage(stepManager.activePage);
                break;
            case 95:
                if (sequencerSteps[stepManager.activePage] == 4)
                {
                    sequencerSteps[stepManager.activePage] = 8;
                    setLed(95, Color::Red);
                }
                else
                {
                    sequencerSteps[stepManager.activePage] = 4;
                    setLed(95, Color::Off);
                }
                break;
            case 98:
                stepManager.deleteSteps();
                loadPage(stepManager.activePage);
                break;
            default:
                break;
            }
            loadStep(stepManager.activeStep);
        }
        else if (message.getChannel() == 16 && message.getControllerNumber() == 100)
            setToProgrammerMode();
    }
}

void LaunchPad::connectOsc()
{
    if (ip1Selector.getText().length() > 0 &&
        ip2Selector.getText().length() > 0 &&
        ip3Selector.getText().length() > 0 &&
        ip4Selector.getText().length() > 0 &&
        portSelector.getText().length() > 0)
    {
        juce::String address = ip1Selector.getText() + "." + ip2Selector.getText() + "." + ip3Selector.getText() + "." + ip4Selector.getText();
        oscSender.disconnect();
        oscSender.connect(address, std::stoi(portSelector.getText().toStdString()));
    }
}

void LaunchPad::checkIpBlock()
{
    if (ip1Selector.getText().length() > 0 && std::stoi(ip1Selector.getText().toStdString()) > 255)
        ip1Selector.setText("255");
    else if (ip2Selector.getText().length() > 0 && std::stoi(ip2Selector.getText().toStdString()) > 255)
        ip2Selector.setText("255");
    else if (ip3Selector.getText().length() > 0 && std::stoi(ip3Selector.getText().toStdString()) > 255)
        ip3Selector.setText("255");
    else if (ip4Selector.getText().length() > 0 && std::stoi(ip4Selector.getText().toStdString()) > 255)
        ip4Selector.setText("255");
}

void LaunchPad::sendOscMessages() 
{    
    int precursor = (currentStep > 0) ? currentStep - 1 : sequencerSteps[stepManager.activePage] - 1;

    for (int i = 0; i < 80; i++)
    {
        int sequenceNumber = MidiNumberToSequenceNumber(i) + sequenceOffset;
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
    for (int i = 0; i < 24; i++)
    {
        sendOscSequencesMessage(MidiNumberToSequenceNumber(i)+sequenceOffset, 0);
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
            setLed(sequencerPads[currentStep], Color::LightBlue);
        else
            setLed(sequencerPads[currentStep], Color::LightPurple);
    else
        setLed(sequencerPads[currentStep], Color::White);
    
    if (currentStep < sequencerSteps[stepManager.activePage] - 1)
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
    if (currentStep < 4)
        setLed(sequencerPads[currentStep], Color::LightBlue);
    else
        setLed(sequencerPads[currentStep], Color::LightPurple);
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
    midi->sendToOutputs(createSysExMessage("\x00\x20\x29\x02\x0c\x0e\x01", 7));;
    loadPage(stepManager.activePage);

    setLed(17, Color::LightGreen);
    setLed(18, Color::LightGreen, LightMode::Pulse);
    setLed(12, Color::Red);
    setLed(94, Color::Red, LightMode::Pulse);
    setLed(98, Color::Red);

    setLed(11, Color::White);

    unsigned char leftSeq[4] = { 81, 82, 83, 84 };
    unsigned char rightSeq[4] = { 85, 86, 87, 88 };
    setLeds(leftSeq, 4, Color::LightBlue);
    setLeds(rightSeq, 4, Color::LightPurple);

    unsigned char leftAction[12] = { 21, 22, 23, 24, 31, 32, 33, 34, 41, 42, 43, 44 };
    unsigned char rightAction[12] = { 25, 26, 27, 28, 35, 36, 37, 38, 45, 46, 47, 48 };
    setLeds(leftAction, 12, Color::Orange);
    setLeds(rightAction, 12, Color::LightYellow);
}

void LaunchPad::setToLiveMode()
{
    //midi->sendToOutputs(createSysExMessage("\x00\x20\x29\x02\x0c\x0e\x00", 7));
    midi->sendToOutputs(createSysExMessage("\x00\x20\x29\x02\x0c\x00\x05", 7));
}
void LaunchPad::changeSequenceOffset()
{
    sequenceOffset = std::stoi(offsetSelector.getText().toStdString());
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

    for (int row = 5; row < 8; row++) {
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
    setLeds(ledsOff, ledsOffCount, Color::LightGreen, LightMode::Static);
}
void LaunchPad::loadPage(int page)
{
    switch (page)
    {
    case 0:
        setLed(89, Color::Red);
        setLed(79, Color::Off);
        setLed(69, Color::Off);
        setLed(59, Color::Off);
        break;
    case 1:
        setLed(89, Color::Off);
        setLed(79, Color::Red);
        setLed(69, Color::Off);
        setLed(59, Color::Off);
        break;
    case 2:
        setLed(89, Color::Off);
        setLed(79, Color::Off);
        setLed(69, Color::Red);
        setLed(59, Color::Off);
        break;
    case 3:
        offAllSequences();
        setLed(89, Color::Off);
        setLed(79, Color::Off);
        setLed(69, Color::Off);
        setLed(59, Color::Red);
        break;
    default:
        break;
    }
    if (page != stepManager.activePage)
        offAllSequences();
        
    stepManager.changePage(page);
    loadStep(stepManager.activeStep);

    if (sequencerSteps[stepManager.activePage] == 4)
        setLed(95, Color::Off);
    else
        setLed(95, Color::Red);
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
    /*
    juce::Line<float> line(juce::Point<float>(0, 400), juce::Point<float>(getWidth(), 400));
    g.setColour(juce::Colours::palegreen);
    g.drawLine(line, 2.0f);
    */
}

void LaunchPad::resized()
{
    auto margin = 10;
    midi->setBounds(0, 0, getWidth(), getHeight());
    startButton.setBounds(margin, (getHeight() / 2), getWidth()/2 - (2 * margin), 24);
    liveButton.setBounds((getWidth()/2 + margin), (getHeight() / 2), getWidth() / 2 - (2 * margin), 24);
    ip1Selector.setBounds(margin, (getHeight() / 2 + 24 + margin), getWidth() / 5 - (2 * margin), 24);
    ip2Selector.setBounds((getWidth() / 5 + margin), (getHeight() / 2 + 24 + margin), getWidth() / 5 - (2 * margin), 24);
    ip3Selector.setBounds((getWidth() / 5 * 2 + margin), (getHeight() / 2 + 24 + margin), getWidth() / 5 - (2 * margin), 24);
    ip4Selector.setBounds((getWidth() / 5 * 3 + margin), (getHeight() / 2 + 24 + margin), getWidth() / 5 - (2 * margin), 24);
    portSelector.setBounds((getWidth() / 5 * 4 + margin), (getHeight() / 2 + 24 + margin), getWidth() / 5 - (2 * margin), 24);
    offsetSelector.setBounds(margin, (getHeight() / 2 + 48 + (2 * margin)), getWidth() / 5 - (2 * margin), 24);
    connectButton.setBounds((getWidth() / 5 + margin), (getHeight() / 2 + 48 + (2 * margin)), getWidth() / 5 - (2 * margin), 24);
}