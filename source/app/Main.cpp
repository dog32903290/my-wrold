#include "MainComponent.h"

#include <juce_gui_extra/juce_gui_extra.h>

namespace myworld
{
namespace
{
juce::String displayName()
{
    return juce::String::fromUTF8 ("\xe6\x88\x91\xe7\x9a\x84\xe4\xb8\x96\xe7\x95\x8c");
}
}

class MyWorldApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return displayName(); }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise (const juce::String& commandLine) override
    {
        const auto dumpProofAndExit = commandLine.contains ("--dump-proof-and-exit");
        const auto dumpAudioProofAndExit = commandLine.contains ("--dump-audio-proof-and-exit");
        const auto quitAfterStartupDump = dumpProofAndExit || dumpAudioProofAndExit;

        mainWindow = std::make_unique<MainWindow> (getApplicationName(),
                                                   dumpProofAndExit,
                                                   dumpAudioProofAndExit,
                                                   quitAfterStartupDump);
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    class MainWindow final : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name,
                    bool dumpProofAndExit,
                    bool dumpAudioProofAndExit,
                    bool quitAfterStartupDump)
            : DocumentWindow (std::move (name),
                              juce::Colour::fromRGB (13, 15, 20),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent (dumpProofAndExit,
                                                dumpAudioProofAndExit,
                                                quitAfterStartupDump),
                             true);
            centreWithSize (getWidth(), getHeight());
            setResizable (true, true);
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
};
}

START_JUCE_APPLICATION (myworld::MyWorldApplication)
