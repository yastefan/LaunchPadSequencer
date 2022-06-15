/*
  ==============================================================================

    MidiComponent.cpp
    Created: 13 Jun 2022 2:06:35pm
    Author:  Stefan

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MidiComponent.h"

//==============================================================================
MidiComponent::MidiComponent(juce::MidiInputCallback* callback) :
    midiInputSelector(new MidiDeviceListBox("Midi Input Selector", *this, true)),
    midiOutputSelector(new MidiDeviceListBox("Midi Output Selector", *this, false))
{
    midiCallback = callback;
    updateDeviceList(true);
    updateDeviceList(false);

    addAndMakeVisible(midiInputLabel);
    addAndMakeVisible(midiOutputLabel);
    addAndMakeVisible(refreshButton);
    addAndMakeVisible(midiInputSelector.get());
    addAndMakeVisible(midiOutputSelector.get());

    refreshButton.onClick = [this]
    {
        updateDeviceList(true);
        updateDeviceList(false);
    };

    setSize(732, 520);
}

MidiComponent::~MidiComponent()
{
    midiInputs.clear();
    midiOutputs.clear();

    midiInputSelector.reset();
    midiOutputSelector.reset();
}

void MidiComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
}

void MidiComponent::resized()
{
    auto margin = 10;
    midiInputLabel.setBounds(margin, margin,
        (getWidth() / 2) - (2 * margin), 24);
    midiOutputLabel.setBounds((getWidth() / 2) + margin, margin,
        (getWidth() / 2) - (2 * margin), 24);
    midiInputSelector->setBounds(margin, (2 * margin) + 24,
        (getWidth() / 2) - (2 * margin),
        (getHeight() / 2) - ((4 * margin) + 24 + 24));
    midiOutputSelector->setBounds((getWidth() / 2) + margin, (2 * margin) + 24,
        (getWidth() / 2) - (2 * margin),
        (getHeight() / 2) - ((4 * margin) + 24 + 24));
    refreshButton.setBounds(margin, (getHeight() / 2) - (margin + 24),
        getWidth() - (2 * margin), 24);

}

void MidiComponent::openDevice(bool isInput, int index)
{
    if (isInput)
    {
        midiInputs[index]->inDevice = juce::MidiInput::openDevice(midiInputs[index]->deviceInfo.identifier, midiCallback);

        if (midiInputs[index]->inDevice.get() == nullptr)
        {
            DBG("MidiDemo::openDevice: open input device for index = " << index << " failed!");
            return;
        }

        midiInputs[index]->inDevice->start();
    }
    else
    {
        midiOutputs[index]->outDevice = juce::MidiOutput::openDevice(midiOutputs[index]->deviceInfo.identifier);

        if (midiOutputs[index]->outDevice.get() == nullptr)
        {
            DBG("MidiDemo::openDevice: open output device for index = " << index << " failed!");
        }
    }
}

void MidiComponent::closeDevice(bool isInput, int index)
{
    if (isInput)
    {
        if (midiInputs[index]->inDevice.get() != nullptr) 
        {
            midiInputs[index]->inDevice->stop();
        }
        midiInputs[index]->inDevice.reset();
    }
    else
    {
        midiOutputs[index]->outDevice.reset();
    }
}

int MidiComponent::getNumMidiInputs() const noexcept
{
    return midiInputs.size();
}

int MidiComponent::getNumMidiOutputs() const noexcept
{
    return midiOutputs.size();
}

juce::ReferenceCountedObjectPtr<MidiDeviceListEntry> MidiComponent::getMidiDevice(int index, bool isInput) const noexcept
{
    return isInput ? midiInputs[index] : midiOutputs[index];
}

void MidiComponent::sendToOutputs(const juce::MidiMessage& msg)
{
    for (auto midiOutput : midiOutputs)
        if (midiOutput->outDevice.get() != nullptr)
            midiOutput->outDevice->sendMessageNow(msg);
}

bool MidiComponent::hasDeviceListChanged(const juce::Array<juce::MidiDeviceInfo>& availableDevices, bool isInputDevice)
{
    juce::ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
        : midiOutputs;

    if (availableDevices.size() != midiDevices.size())
        return true;

    for (auto i = 0; i < availableDevices.size(); ++i)
        if (availableDevices[i] != midiDevices[i]->deviceInfo)
            return true;

    return false;
}

juce::ReferenceCountedObjectPtr<MidiDeviceListEntry> MidiComponent::findDevice(juce::MidiDeviceInfo device, bool isInputDevice) const
{
    const juce::ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
        : midiOutputs;

    for (auto& d : midiDevices)
        if (d->deviceInfo == device)
            return d;

    return nullptr;
}

void MidiComponent::closeUnpluggedDevices(const juce::Array<juce::MidiDeviceInfo>& currentlyPluggedInDevices, bool isInputDevice)
{
    juce::ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = isInputDevice ? midiInputs
        : midiOutputs;

    for (auto i = midiDevices.size(); --i >= 0;)
    {
        auto& d = *midiDevices[i];

        if (!currentlyPluggedInDevices.contains(d.deviceInfo))
        {
            if (isInputDevice ? d.inDevice.get() != nullptr
                : d.outDevice.get() != nullptr)
                closeDevice(isInputDevice, i);

            midiDevices.remove(i);
        }
    }
}

void MidiComponent::updateDeviceList(bool isInputDeviceList)
{
    auto availableDevices = isInputDeviceList ? juce::MidiInput::getAvailableDevices()
        : juce::MidiOutput::getAvailableDevices();

    if (hasDeviceListChanged(availableDevices, isInputDeviceList))
    {

       juce::ReferenceCountedArray<MidiDeviceListEntry>& midiDevices
            = isInputDeviceList ? midiInputs : midiOutputs;

        closeUnpluggedDevices(availableDevices, isInputDeviceList);

        juce::ReferenceCountedArray<MidiDeviceListEntry> newDeviceList;
        // add all currently plugged-in devices to the device list
        for (auto& newDevice : availableDevices)
        {
            MidiDeviceListEntry::Ptr entry = findDevice(newDevice, isInputDeviceList);

            if (entry == nullptr)
                entry = new MidiDeviceListEntry(newDevice);

            newDeviceList.add(entry);
        }

        // actually update the device list
        midiDevices = newDeviceList;

        // update the selection status of the combo-box
        if (auto* midiSelector = isInputDeviceList ? midiInputSelector.get() : midiOutputSelector.get())
            midiSelector->syncSelectedItemsWithDeviceList(midiDevices);
    }
}

//==============================================================================

MidiComponent::MidiDeviceListBox::MidiDeviceListBox(const juce::String& name, MidiComponent& contentComponent, bool isInputDeviceList):
    ListBox(name, this),
    parent(contentComponent),
    isInput(isInputDeviceList)
{
    setOutlineThickness(1);
    setMultipleSelectionEnabled(false);
    setClickingTogglesRowSelection(true);
}

int MidiComponent::MidiDeviceListBox::getNumRows()
{
    return isInput ? parent.getNumMidiInputs()
        : parent.getNumMidiOutputs();
}

void MidiComponent::MidiDeviceListBox::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    auto textColour = getLookAndFeel().findColour(ListBox::textColourId);

    if (rowIsSelected) g.fillAll(textColour.interpolatedWith(getLookAndFeel().findColour(ListBox::backgroundColourId), 0.5));

    g.setColour(textColour);
    g.setFont((float)height * 0.7f);

    if (isInput)
    {
        if (rowNumber < parent.getNumMidiInputs())
        {
            g.drawText(parent.getMidiDevice(rowNumber, true)->deviceInfo.name,
                5, 0, width, height,
                juce::Justification::centredLeft, true);
        }
    }
    else
    {
        if (rowNumber < parent.getNumMidiOutputs())
        {
            g.drawText(parent.getMidiDevice(rowNumber, false)->deviceInfo.name,
                5, 0, width, height,
                juce::Justification::centredLeft, true);
        }
    }
}

void MidiComponent::MidiDeviceListBox::selectedRowsChanged(int)
{
    auto newSelectedItems = getSelectedRows();
    if (newSelectedItems != lastSelectedItems)
    {
        for (auto i = 0; i < lastSelectedItems.size(); ++i)
        {
            if (!newSelectedItems.contains(lastSelectedItems[i]))
                parent.closeDevice(isInput, lastSelectedItems[i]);
        }

        for (auto i = 0; i < newSelectedItems.size(); ++i)
        {
            if (!lastSelectedItems.contains(newSelectedItems[i]))
                parent.openDevice(isInput, newSelectedItems[i]);
        }

        lastSelectedItems = newSelectedItems;
    }
}

void MidiComponent::MidiDeviceListBox::syncSelectedItemsWithDeviceList(const juce::ReferenceCountedArray<MidiDeviceListEntry>& midiDevices)
{
    juce::SparseSet<int> selectedRows;
    for (auto i = 0; i < midiDevices.size(); ++i)
    {
        if (midiDevices[i]->inDevice.get() != nullptr || midiDevices[i]->outDevice.get() != nullptr)
            selectedRows.addRange(juce::Range<int>(i, i + 1));
    }
    lastSelectedItems = selectedRows;
    updateContent();
    setSelectedRows(selectedRows, juce::dontSendNotification);
}
