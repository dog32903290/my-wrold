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

StartupProofOptions startupProofOptionsFromCommandLine (const juce::String& commandLine)
{
    StartupProofOptions options;
    options.dumpV1ShaderProof = commandLine.contains ("--dump-proof-and-exit");
    options.dumpA1AudioProof = commandLine.contains ("--dump-audio-proof-and-exit");
    options.dumpC2StorageProof = commandLine.contains ("--dump-c2-storage-proof-and-exit");
    options.dumpC3SaveWorkProof = commandLine.contains ("--dump-c3-save-work-proof-and-exit");
    options.dumpC4AIWorkerSaveWorkProof = commandLine.contains ("--dump-c4-ai-worker-save-work-proof-and-exit");
    options.dumpC5ModulePublishProof = commandLine.contains ("--dump-c5-module-publish-proof-and-exit");
    options.dumpC5AIWorkerModulePublishProof = commandLine.contains (
        "--dump-c5-ai-worker-module-publish-proof-and-exit");
    options.dumpC5VisibleModulePublishProof = commandLine.contains (
        "--dump-c5-visible-module-publish-proof-and-exit");
    options.dumpC6AnalyzerFamilyProof = commandLine.contains ("--dump-c6-analyzer-family-proof-and-exit");
    options.dumpC6AIRepairLoopProof = commandLine.contains ("--dump-c6-ai-repair-loop-proof-and-exit");
    options.dumpPVAttackDetectorProof = commandLine.contains ("--dump-pv-attack-detector-proof-and-exit");
    options.dumpPVDensityDetectorProof = commandLine.contains ("--dump-pv-density-detector-proof-and-exit");
    options.dumpPVSilenceDetectorProof = commandLine.contains ("--dump-pv-silence-detector-proof-and-exit");
    options.dumpPVSustainDetectorProof = commandLine.contains ("--dump-pv-sustain-detector-proof-and-exit");
    options.dumpPVResidueDetectorProof = commandLine.contains ("--dump-pv-residue-detector-proof-and-exit");
    options.dumpPVAggregatePressureProof = commandLine.contains (
        "--dump-pv-aggregate-pressure-proof-and-exit");
    options.dumpPVB1AnalyzerEnvironmentProof = commandLine.contains (
        "--dump-pv-b1-analyzer-environment-proof-and-exit");
    options.quitAfterStartupDump = hasStartupProofRequest (options);

    return options;
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
        mainWindow = std::make_unique<MainWindow> (getApplicationName(),
                                                   startupProofOptionsFromCommandLine (commandLine));
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
        MainWindow (juce::String name, StartupProofOptions startupProofOptions)
            : DocumentWindow (std::move (name),
                              juce::Colour::fromRGB (13, 15, 20),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent (startupProofOptions),
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
