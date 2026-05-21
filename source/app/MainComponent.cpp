#include "MainComponent.h"

#include "GraphContract.h"

namespace myworld
{
namespace
{
juce::Font monoFont (float height)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), height, juce::Font::plain));
}
}

MainComponent::MainComponent()
{
    const auto graph = makeDefaultShaderOutputGraph();

    graphLabel.setText (juce::String (graph.runtimeGraph.nodes[0].id) + " -> " + graph.runtimeGraph.nodes[1].id,
                        juce::dontSendNotification);
    graphLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    graphLabel.setFont (monoFont (15.0f));
    addAndMakeVisible (graphLabel);

    statusLabel.setText ("waiting for GL context", juce::dontSendNotification);
    statusLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (172, 184, 204));
    statusLabel.setFont (juce::Font (juce::FontOptions (13.0f)));
    addAndMakeVisible (statusLabel);

    shaderEditor.setMultiLine (true);
    shaderEditor.setReturnKeyStartsNewLine (true);
    shaderEditor.setTabKeyUsedAsCharacter (true);
    shaderEditor.setScrollbarsShown (true);
    shaderEditor.setFont (monoFont (14.0f));
    shaderEditor.setText (defaultFragmentShader(), juce::dontSendNotification);
    shaderEditor.onTextChange = [this]
    {
        preview.setFragmentShader (shaderEditor.getText().toStdString());
    };
    addAndMakeVisible (shaderEditor);

    preview.onStatusMessage = [safe = juce::Component::SafePointer<MainComponent> (this)] (juce::String incomingStatus)
    {
        juce::MessageManager::callAsync ([safe, statusMessage = std::move (incomingStatus)]
        {
            if (safe != nullptr)
                safe->setShaderStatus (statusMessage);
        });
    };
    addAndMakeVisible (preview);

    setSize (1180, 720);
}

MainComponent::~MainComponent()
{
    preview.onStatusMessage = nullptr;
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (13, 15, 20));
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced (14);
    auto header = area.removeFromTop (28);

    graphLabel.setBounds (header.removeFromLeft (220));
    statusLabel.setBounds (header);

    area.removeFromTop (10);

    auto left = area.removeFromLeft (juce::jmax (360, area.getWidth() / 2));
    shaderEditor.setBounds (left.reduced (0, 0));

    area.removeFromLeft (12);
    preview.setBounds (area);
}

void MainComponent::setShaderStatus (juce::String message)
{
    statusLabel.setText (std::move (message), juce::dontSendNotification);
}
}
