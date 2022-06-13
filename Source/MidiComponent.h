/*
  ==============================================================================

    MidiComponent.h
    Created: 13 Jun 2022 2:06:35pm
    Author:  Stefan

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
struct MidiDeviceListEntry : juce::ReferenceCountedObject
{
    MidiDeviceListEntry(juce::MidiDeviceInfo info) : deviceInfo(info) {}

    juce::MidiDeviceInfo deviceInfo;
    std::unique_ptr<juce::MidiInput> inDevice;
    std::unique_ptr<juce::MidiOutput> outDevice;

    using Ptr = juce::ReferenceCountedObjectPtr<MidiDeviceListEntry>;
};

class MidiComponent  : public juce::Component,
                       public juce::MidiInputCallback
{
public:
    MidiComponent();
    ~MidiComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void openDevice(bool, int);
    void closeDevice(bool, int);

    int getNumMidiInputs() const noexcept;
    int getNumMidiOutputs() const noexcept;
    juce::ReferenceCountedObjectPtr<MidiDeviceListEntry> getMidiDevice(int index, bool isInput) const noexcept;

private:
    class MidiDeviceListBox : public juce::ListBox,
        private juce::ListBoxModel
    {
    public:
        MidiDeviceListBox(const juce::String&, MidiComponent&, bool);
        int getNumRows() override;
        void paintListBoxItem(int, juce::Graphics& g, int, int, bool) override;
        void selectedRowsChanged(int) override;
        void syncSelectedItemsWithDeviceList(const juce::ReferenceCountedArray<MidiDeviceListEntry>&);

    private:
        MidiComponent& parent;
        bool isInput;
        juce::SparseSet<int> lastSelectedItems;
    };

    std::unique_ptr<MidiDeviceListBox> midiInputSelector, midiOutputSelector;
    juce::ReferenceCountedArray<MidiDeviceListEntry> midiInputs, midiOutputs;

    void handleIncomingMidiMessage(juce::MidiInput*, const juce::MidiMessage&) override;
    void sendToOutputs(const juce::MidiMessage&);
    bool hasDeviceListChanged(const juce::Array<juce::MidiDeviceInfo>&, bool);
    juce::ReferenceCountedObjectPtr<MidiDeviceListEntry> findDevice(juce::MidiDeviceInfo, bool) const;
    void closeUnpluggedDevices(const juce::Array<juce::MidiDeviceInfo>&, bool);
    void updateDeviceList(bool);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiComponent)
};

