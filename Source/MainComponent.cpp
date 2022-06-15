#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize(732, 520);
    addAndMakeVisible(launchPad);
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
    launchPad.setBounds(getLocalBounds());
}

void MainComponent::timerCallback()
{

}

