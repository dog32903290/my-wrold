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
        const auto dumpC2StorageProofAndExit = commandLine.contains ("--dump-c2-storage-proof-and-exit");
        const auto dumpC3SaveWorkProofAndExit = commandLine.contains ("--dump-c3-save-work-proof-and-exit");
        const auto dumpC4AIWorkerSaveWorkProofAndExit = commandLine.contains ("--dump-c4-ai-worker-save-work-proof-and-exit");
        const auto dumpC5ModulePublishProofAndExit = commandLine.contains ("--dump-c5-module-publish-proof-and-exit");
        const auto dumpC5AIWorkerModulePublishProofAndExit = commandLine.contains (
            "--dump-c5-ai-worker-module-publish-proof-and-exit");
        const auto dumpC5VisibleModulePublishProofAndExit = commandLine.contains (
            "--dump-c5-visible-module-publish-proof-and-exit");
        const auto dumpC6AnalyzerFamilyProofAndExit = commandLine.contains (
            "--dump-c6-analyzer-family-proof-and-exit");
        const auto dumpC6AIRepairLoopProofAndExit = commandLine.contains (
            "--dump-c6-ai-repair-loop-proof-and-exit");
        const auto quitAfterStartupDump = dumpProofAndExit || dumpAudioProofAndExit || dumpC2StorageProofAndExit
                                          || dumpC3SaveWorkProofAndExit || dumpC4AIWorkerSaveWorkProofAndExit
                                          || dumpC5ModulePublishProofAndExit || dumpC5AIWorkerModulePublishProofAndExit
                                          || dumpC5VisibleModulePublishProofAndExit
                                          || dumpC6AnalyzerFamilyProofAndExit
                                          || dumpC6AIRepairLoopProofAndExit;

        mainWindow = std::make_unique<MainWindow> (getApplicationName(),
                                                   dumpProofAndExit,
                                                   dumpAudioProofAndExit,
                                                   dumpC2StorageProofAndExit,
                                                   dumpC3SaveWorkProofAndExit,
                                                   dumpC4AIWorkerSaveWorkProofAndExit,
                                                   dumpC5ModulePublishProofAndExit,
                                                   dumpC5AIWorkerModulePublishProofAndExit,
                                                   dumpC5VisibleModulePublishProofAndExit,
                                                   dumpC6AnalyzerFamilyProofAndExit,
                                                   dumpC6AIRepairLoopProofAndExit,
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
                    bool dumpC2StorageProofAndExit,
                    bool dumpC3SaveWorkProofAndExit,
                    bool dumpC4AIWorkerSaveWorkProofAndExit,
                    bool dumpC5ModulePublishProofAndExit,
                    bool dumpC5AIWorkerModulePublishProofAndExit,
                    bool dumpC5VisibleModulePublishProofAndExit,
                    bool dumpC6AnalyzerFamilyProofAndExit,
                    bool dumpC6AIRepairLoopProofAndExit,
                    bool quitAfterStartupDump)
            : DocumentWindow (std::move (name),
                              juce::Colour::fromRGB (13, 15, 20),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent (dumpProofAndExit,
                                                dumpAudioProofAndExit,
                                                dumpC2StorageProofAndExit,
                                                dumpC3SaveWorkProofAndExit,
                                                dumpC4AIWorkerSaveWorkProofAndExit,
                                                dumpC5ModulePublishProofAndExit,
                                                dumpC5AIWorkerModulePublishProofAndExit,
                                                dumpC5VisibleModulePublishProofAndExit,
                                                dumpC6AnalyzerFamilyProofAndExit,
                                                dumpC6AIRepairLoopProofAndExit,
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
