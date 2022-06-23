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


class Sequencer : private juce::Timer
{
public:
    Sequencer();
    ~Sequencer();

private:
    std::function<void(int)> callbackFunction;
    void timerCallback() final;
};
