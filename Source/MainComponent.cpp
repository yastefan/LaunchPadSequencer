#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize(500, 360);
    addAndMakeVisible(launchPad);
}

MainComponent::~MainComponent()
{
 
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawText("Schmiddis Light Sequencer", 10, 2, getWidth(), 40, juce::Justification::centredLeft, true);
}

void MainComponent::resized()
{
    int margin = 2;
    launchPad.setBounds(margin, 40, getWidth()-(margin*2), getHeight() - 40);
}