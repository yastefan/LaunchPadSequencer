/*
  ==============================================================================

    Osc.cpp
    Created: 3 Aug 2022 1:14:56pm
    Author:  Stefan

  ==============================================================================
*/

#include "Osc.h"

Osc::Osc() {
    connect("127.0.0.1", 9005);
}

Osc::~Osc() {
    disconnect();
}

void Osc::sendGrandMA3Sequence(int sequenceNumber, int value, bool swop) {
    std::string message_address = "/13.13.1.5." + std::to_string(sequenceNumber);

    juce::String b = swop ? "Swop" : "Temp";
    juce::OSCArgument button(b);

    send(message_address.c_str(), button, value);
}

void Osc::sendGenericOsc(int sequenceNumber, int value) {
    std::string message_address = "/sequencer/step/" + std::to_string(sequenceNumber);
    send(message_address.c_str(), value);
}