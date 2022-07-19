#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize(500, 340);
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
    int margin = 2;
    launchPad.setBounds(margin, 0, getWidth()-(margin*2), getHeight());
}