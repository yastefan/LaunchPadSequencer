/*
  ==============================================================================

    Osc.h
    Created: 3 Aug 2022 1:14:56pm
    Author:  Stefan

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Osc : public juce::OSCSender
{
public:
    Osc();
    ~Osc();

private:
    void sendGrandMA3Sequence(int sequenceNumber, int value, bool swop = false);
    void sendGenericOsc(int sequenceNumber, int value);
};