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
    startTimer(500.0);
}

Sequencer::~Sequencer()
{
}

void Sequencer::timerCallback()
{
    DBG("500 ms have passed");
}