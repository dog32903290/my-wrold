#include "PreferencesPanel.h"

namespace myworld
{
namespace
{
juce::Font monoFont (float height)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), height, juce::Font::plain));
}

void configureSlider (juce::Slider& slider, double min, double max, double interval, double value)
{
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 54, 20);
    slider.setRange (min, max, interval);
    slider.setValue (value, juce::dontSendNotification);
}
}

PreferencesPanel::PreferencesPanel (juce::AudioDeviceManager& deviceManager)
    : audioSelector (deviceManager, 1, 2, 0, 0, false, false, false, false)
{
    configureLabel (titleLabel, "Preferences");
    configureLabel (audioLabel, "Audio");
    configureLabel (midiLabel, "MIDI");
    configureLabel (gainLabel, "analysis gain");
    configureLabel (midiOutputLabel, "output");
    configureLabel (midiChannelLabel, "channel");
    configureLabel (loudnessCcLabel, "loudness CC");
    configureLabel (mapCcLabel, "map CC");

    addAndMakeVisible (titleLabel);
    addAndMakeVisible (audioLabel);
    addAndMakeVisible (midiLabel);
    addAndMakeVisible (gainLabel);
    addAndMakeVisible (midiOutputLabel);
    addAndMakeVisible (midiChannelLabel);
    addAndMakeVisible (loudnessCcLabel);
    addAndMakeVisible (mapCcLabel);

    audioSelector.setItemHeight (20);
    addAndMakeVisible (audioSelector);

    configureSlider (analysisGainSlider, 0.0, 8.0, 0.01, 1.0);
    analysisGainSlider.onValueChange = [this] { emitAnalysisGain(); };
    addAndMakeVisible (analysisGainSlider);

    midiOutputBox.onChange = [this] { emitMidiPreferences(); };
    addAndMakeVisible (midiOutputBox);

    configureSlider (midiChannelSlider, 1.0, 16.0, 1.0, 1.0);
    midiChannelSlider.onValueChange = [this] { emitMidiPreferences(); };
    addAndMakeVisible (midiChannelSlider);

    configureSlider (loudnessCcSlider, 0.0, 127.0, 1.0, 20.0);
    loudnessCcSlider.onValueChange = [this] { emitMidiPreferences(); };
    addAndMakeVisible (loudnessCcSlider);

    configureSlider (mapCcSlider, 0.0, 127.0, 1.0, 20.0);
    mapCcSlider.onValueChange = [this] { emitMidiPreferences(); };
    addAndMakeVisible (mapCcSlider);

    midiStreamButton.setButtonText ("MIDI Stream");
    midiStreamButton.onClick = [this] { emitMidiPreferences(); };
    addAndMakeVisible (midiStreamButton);

    mapModeButton.setButtonText ("Map Mode");
    mapModeButton.onClick = [this] { emitMidiPreferences(); };
    addAndMakeVisible (mapModeButton);

    refreshMidiButton.setButtonText ("Refresh MIDI");
    refreshMidiButton.onClick = [this] { refreshMidiOutputs(); };
    addAndMakeVisible (refreshMidiButton);

    refreshMidiOutputs();
}

float PreferencesPanel::getAnalysisGain() const
{
    return static_cast<float> (analysisGainSlider.getValue());
}

MidiPreferences PreferencesPanel::getMidiPreferences() const
{
    MidiPreferences preferences;
    preferences.streamEnabled = midiStreamButton.getToggleState();
    preferences.mapModeEnabled = mapModeButton.getToggleState();
    preferences.channel = juce::roundToInt (midiChannelSlider.getValue());
    preferences.loudnessCc = juce::roundToInt (loudnessCcSlider.getValue());
    preferences.mapCc = juce::roundToInt (mapCcSlider.getValue());

    const auto selectedId = midiOutputBox.getSelectedId();

    if (selectedId > 1)
    {
        const auto index = selectedId - 2;

        if (index >= 0 && index < static_cast<int> (midiOutputs.size()))
        {
            preferences.outputIdentifier = midiOutputs[static_cast<size_t> (index)].identifier.toStdString();
            preferences.outputName = midiOutputs[static_cast<size_t> (index)].name.toStdString();
        }
    }

    return sanitizePerformancePreferences ({ {}, preferences }).midi;
}

void PreferencesPanel::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (18, 21, 27));
    g.setColour (juce::Colour::fromRGB (56, 63, 73));
    g.drawRect (getLocalBounds(), 1);
}

void PreferencesPanel::resized()
{
    auto area = getLocalBounds().reduced (10);
    titleLabel.setBounds (area.removeFromTop (20));
    area.removeFromTop (6);

    auto audioArea = area.removeFromLeft (juce::jmin (430, area.getWidth() / 2));
    area.removeFromLeft (14);

    audioLabel.setBounds (audioArea.removeFromTop (18));
    audioSelector.setBounds (audioArea);

    midiLabel.setBounds (area.removeFromTop (18));
    area.removeFromTop (4);

    auto row = area.removeFromTop (24);
    gainLabel.setBounds (row.removeFromLeft (104));
    analysisGainSlider.setBounds (row);

    area.removeFromTop (4);
    row = area.removeFromTop (24);
    midiOutputLabel.setBounds (row.removeFromLeft (74));
    refreshMidiButton.setBounds (row.removeFromRight (112));
    row.removeFromRight (8);
    midiOutputBox.setBounds (row);

    area.removeFromTop (4);
    row = area.removeFromTop (24);
    midiStreamButton.setBounds (row.removeFromLeft (128));
    mapModeButton.setBounds (row.removeFromLeft (116));

    area.removeFromTop (4);
    row = area.removeFromTop (24);
    midiChannelLabel.setBounds (row.removeFromLeft (74));
    midiChannelSlider.setBounds (row.removeFromLeft (150));
    row.removeFromLeft (10);
    loudnessCcLabel.setBounds (row.removeFromLeft (96));
    loudnessCcSlider.setBounds (row);

    area.removeFromTop (4);
    row = area.removeFromTop (24);
    mapCcLabel.setBounds (row.removeFromLeft (74));
    mapCcSlider.setBounds (row.removeFromLeft (150));
}

void PreferencesPanel::refreshMidiOutputs()
{
    const auto previousIdentifier = getMidiPreferences().outputIdentifier;

    midiOutputs.clear();
    const auto outputs = juce::MidiOutput::getAvailableDevices();

    for (const auto& output : outputs)
        midiOutputs.push_back (output);

    midiOutputBox.clear (juce::dontSendNotification);
    midiOutputBox.addItem ("None", 1);

    int selectedId = 1;

    for (int index = 0; index < static_cast<int> (midiOutputs.size()); ++index)
    {
        const auto id = index + 2;
        midiOutputBox.addItem (midiOutputs[static_cast<size_t> (index)].name, id);

        if (midiOutputs[static_cast<size_t> (index)].identifier.toStdString() == previousIdentifier)
            selectedId = id;
    }

    midiOutputBox.setSelectedId (selectedId, juce::dontSendNotification);
    emitMidiPreferences();
}

void PreferencesPanel::emitAnalysisGain()
{
    if (onAnalysisGainChanged != nullptr)
        onAnalysisGainChanged (getAnalysisGain());
}

void PreferencesPanel::emitMidiPreferences()
{
    if (onMidiPreferencesChanged != nullptr)
        onMidiPreferencesChanged (getMidiPreferences());
}

void PreferencesPanel::configureLabel (juce::Label& label, const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
    label.setColour (juce::Label::textColourId, juce::Colour::fromRGB (203, 211, 226));
    label.setFont (monoFont (13.0f));
}
}
