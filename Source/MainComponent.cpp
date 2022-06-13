#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize(732, 520);
    addAndMakeVisible(midiComponent);
}

MainComponent::~MainComponent()
{
 
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{

}

void MainComponent::resized()
{
    midiComponent.setBounds(getLocalBounds());
}

void MainComponent::timerCallback()
{

}

