#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize(500, 320);
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