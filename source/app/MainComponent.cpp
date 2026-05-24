#include "MainComponent.h"

#include "C4AIWorkerSaveWorkProofRunner.h"
#include "C6AIRepairLoopProofRunner.h"
#include "C6AnalyzerFamilyProofRunner.h"
#include "C5ModulePublishProofRunner.h"
#include "CompoundModule.h"
#include "CompoundPatch.h"
#include "GraphEndpoint.h"
#include "GraphContract.h"
#include "InteractionContract.h"
#include "PVB1AnalyzerEnvironmentProofRunner.h"
#include "PVDetectorProofRunner.h"
#include "ProofReports.h"
#include "RuntimeRegistry.h"
#include "StorageCommand.h"
#include "StorageContract.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <utility>
#include <vector>

namespace myworld
{
namespace
{
juce::Font monoFont (float height)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), height, juce::Font::plain));
}

bool looksLikeProjectDirectory (const juce::File& directory)
{
    return directory.getChildFile ("CMakeLists.txt").existsAsFile()
           && directory.getChildFile ("source").isDirectory()
           && directory.getChildFile ("fixtures").isDirectory();
}

void configureMeterLabel (juce::Label& label, juce::String text)
{
    label.setText (std::move (text), juce::dontSendNotification);
    label.setColour (juce::Label::textColourId, juce::Colour::fromRGB (202, 211, 226));
    label.setFont (monoFont (13.0f));
}

juce::File projectDirectory()
{
    const auto environmentPath = juce::SystemStats::getEnvironmentVariable ("MY_WORLD_PROJECT_DIR", {});

    if (environmentPath.isNotEmpty())
        return juce::File (environmentPath);

    const auto workingDirectory = juce::File::getCurrentWorkingDirectory();

    if (looksLikeProjectDirectory (workingDirectory))
        return workingDirectory;

    return juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
        .getChildFile (juce::String::fromUTF8 ("\xe6\x88\x91\xe7\x9a\x84\xe4\xb8\x96\xe7\x95\x8c"));
}

juce::File proofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("v1-shader-proof");
}

juce::File audioProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("a1-audio-proof");
}

juce::File c2StorageProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c2-storage-proof");
}

juce::File c3SaveWorkProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c3-save-work-proof");
}

juce::File c4AIWorkerSaveWorkProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (c4AIWorkerSaveWorkProofDirectoryName());
}

juce::File c5ModulePublishProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (
        c5ModulePublishProofDirectoryName (C5ModulePublishProofKind::modulePublish));
}

juce::File c5AIWorkerModulePublishProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (
        c5ModulePublishProofDirectoryName (C5ModulePublishProofKind::aiWorkerModulePublish));
}

juce::File c5VisibleModulePublishDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c5-visible-module-publish");
}

juce::File c5VisibleModulePublishProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (
        c5ModulePublishProofDirectoryName (C5ModulePublishProofKind::visibleModulePublish));
}

juce::File c6AnalyzerFamilyProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (c6AnalyzerFamilyProofDirectoryName());
}

juce::File c6AIRepairLoopProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (c6AIRepairLoopProofDirectoryName());
}

juce::File pvDetectorProofDumpDirectory (PVDetectorProofKind kind)
{
    return projectDirectory().getChildFile ("debug").getChildFile (pvDetectorProofDirectoryName (kind));
}

juce::File pvB1AnalyzerEnvironmentProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile (pvB1AnalyzerEnvironmentProofDirectoryName());
}

juce::File defaultActiveWorkManifestFile()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c3-active-work").getChildFile ("myworld.work.json");
}

juce::File activeWorkManifestFile()
{
    const auto environmentPath = juce::SystemStats::getEnvironmentVariable ("MY_WORLD_ACTIVE_WORK_MANIFEST", {});

    if (environmentPath.isNotEmpty())
        return juce::File (environmentPath);

    return defaultActiveWorkManifestFile();
}

juce::File parentDirectory (juce::File file, const int levels)
{
    for (int i = 0; i < levels; ++i)
        file = file.getParentDirectory();

    return file;
}

juce::String defaultModuleLibraryPath()
{
    return "fixtures/module-libraries/default.module-library.json";
}

std::vector<std::string> moduleLibraryCandidatePaths (const juce::String& libraryPath)
{
    const auto executableDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
    const auto buildAppRepoRoot = parentDirectory (executableDir, 5);

    return {
        juce::File::getCurrentWorkingDirectory().getChildFile (libraryPath).getFullPathName().toStdString(),
        buildAppRepoRoot.getChildFile (libraryPath).getFullPathName().toStdString()
    };
}

std::vector<std::string> repoCandidatePaths (const juce::String& relativePath)
{
    return moduleLibraryCandidatePaths (relativePath);
}

std::vector<std::filesystem::path> proofCandidateRoots()
{
    const auto executableDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();

    return {
        projectDirectory().getFullPathName().toStdString(),
        juce::File::getCurrentWorkingDirectory().getFullPathName().toStdString(),
        parentDirectory (executableDir, 5).getFullPathName().toStdString()
    };
}

bool hasEdgeId (const GraphContract& graph, const std::string& edgeId)
{
    return std::any_of (graph.editorGraph.edges.begin(),
                        graph.editorGraph.edges.end(),
                        [&edgeId] (const auto& edge) {
                            return edge.id == edgeId;
                        });
}

bool nearlyEqual (double lhs, double rhs)
{
    return std::abs (lhs - rhs) < 0.000001;
}

bool writeTextFile (const juce::File& file, const std::string& text)
{
    return file.replaceWithText (juce::String::fromUTF8 (text.c_str()), false, false, "\n");
}

std::string clearDirectoryIfExists (const juce::File& directory)
{
    if (directory.exists() && ! directory.deleteRecursively())
        return "could not clear " + directory.getFullPathName().toStdString();

    return {};
}

std::string createDirectoryIfMissing (const juce::File& directory)
{
    if (! directory.createDirectory())
        return "could not create " + directory.getFullPathName().toStdString();

    return {};
}

bool copyTextFile (const juce::File& source, const juce::File& target)
{
    if (! target.getParentDirectory().createDirectory())
        return false;

    if (target.existsAsFile() && ! target.deleteFile())
        return false;

    return source.copyFileTo (target);
}

bool copyFirstRepoCandidate (const juce::String& relativePath, const juce::File& target, std::string& error)
{
    for (const auto& candidate : repoCandidatePaths (relativePath))
    {
        const auto source = juce::File (candidate);
        if (copyTextFile (source, target))
            return true;

        error = "could not copy " + relativePath.toStdString() + " from " + candidate;
    }

    if (error.empty())
        error = "could not copy " + relativePath.toStdString();

    return false;
}

bool prepareDefaultActiveWorkProject (const juce::File& workManifestFile, std::string& error)
{
    const auto patchFile = workManifestFile.getParentDirectory().getChildFile ("patches").getChildFile ("main.patch.json");
    const auto debugDirectory = workManifestFile.getParentDirectory().getParentDirectory();
    const auto moduleLibraryFile = debugDirectory.getChildFile ("module-libraries").getChildFile ("default.module-library.json");
    const auto moduleManifestFile = debugDirectory.getChildFile ("module-libraries")
                                          .getChildFile ("modules")
                                          .getChildFile ("loudness")
                                          .getChildFile ("module.json");

    if (! workManifestFile.existsAsFile()
        && ! copyFirstRepoCandidate ("fixtures/storage/c2-compound-work/myworld.work.json", workManifestFile, error))
    {
        return false;
    }

    if (! patchFile.existsAsFile()
        && ! copyFirstRepoCandidate ("fixtures/storage/c2-compound-work/patches/main.patch.json", patchFile, error))
    {
        return false;
    }

    if (! moduleLibraryFile.existsAsFile()
        && ! copyFirstRepoCandidate ("fixtures/module-libraries/default.module-library.json", moduleLibraryFile, error))
    {
        return false;
    }

    if (! moduleManifestFile.existsAsFile()
        && ! copyFirstRepoCandidate ("fixtures/modules/loudness/module.json", moduleManifestFile, error))
    {
        return false;
    }

    return true;
}

std::string safeIdentifier (const std::string& text)
{
    std::string result;
    result.reserve (text.size());

    for (const auto character : text)
    {
        const auto byte = static_cast<unsigned char> (character);
        if (std::isalnum (byte) != 0 || character == '-' || character == '_')
            result.push_back (static_cast<char> (std::tolower (byte)));
        else
            result.push_back ('-');
    }

    return result.empty() ? "module" : result;
}

RuntimeRegistryLoadResult loadAudioProofRuntimeRegistry()
{
    std::string lastError;

    for (const auto& path : moduleLibraryCandidatePaths (defaultModuleLibraryPath()))
    {
        const auto registry = loadRuntimeRegistryFromModuleLibrary (path);
        if (registry.ok)
            return registry;

        lastError = registry.error;
    }

    return { false, {}, lastError.empty() ? "could not load module library: " + defaultModuleLibraryPath().toStdString()
                                          : lastError };
}

}

MainComponent::MainComponent (bool dumpProofOnStart,
                              bool dumpAudioProofOnStart,
                              bool dumpC2StorageProofOnStart,
                              bool dumpC3SaveWorkProofOnStart,
                              bool dumpC4AIWorkerSaveWorkProofOnStart,
                              bool dumpC5ModulePublishProofOnStart,
                              bool dumpC5AIWorkerModulePublishProofOnStart,
                              bool dumpC5VisibleModulePublishProofOnStart,
                              bool dumpC6AnalyzerFamilyProofOnStart,
                              bool dumpC6AIRepairLoopProofOnStart,
                              bool dumpPVAttackDetectorProofOnStart,
                              bool dumpPVDensityDetectorProofOnStart,
                              bool dumpPVSilenceDetectorProofOnStart,
                              bool dumpPVSustainDetectorProofOnStart,
                              bool dumpPVResidueDetectorProofOnStart,
                              bool dumpPVAggregatePressureProofOnStart,
                              bool dumpPVB1AnalyzerEnvironmentProofOnStart,
                              bool quitAfterStartupDump)
    : preferencesPanel (audioDeviceManager),
      graph (makeDefaultShaderOutputGraph()),
      performancePreferences (makeDefaultPerformancePreferences()),
      shouldQuitAfterStartupDump (quitAfterStartupDump)
{
    graphLabel.setText ("node canvas workspace",
                        juce::dontSendNotification);
    graphLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    graphLabel.setFont (monoFont (15.0f));
    addAndMakeVisible (graphLabel);

    statusLabel.setText ("waiting for GL context", juce::dontSendNotification);
    statusLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (172, 184, 204));
    statusLabel.setFont (juce::Font (juce::FontOptions (13.0f)));
    addAndMakeVisible (statusLabel);

    audioStatusLabel.setText ("audio input starting", juce::dontSendNotification);
    audioStatusLabel.setColour (juce::Label::textColourId, juce::Colour::fromRGB (157, 198, 218));
    audioStatusLabel.setFont (monoFont (13.0f));
    addAndMakeVisible (audioStatusLabel);

    configureMeterLabel (rmsLabel, "rms 0.0000");
    configureMeterLabel (peakLabel, "peak 0.0000");
    configureMeterLabel (loudnessLabel, "loudness 0.0000");
    configureMeterLabel (activeLabel, "active no");
    configureMeterLabel (midiStatusLabel, "midi off");
    addAndMakeVisible (rmsLabel);
    addAndMakeVisible (peakLabel);
    addAndMakeVisible (loudnessLabel);
    addAndMakeVisible (activeLabel);
    addAndMakeVisible (midiStatusLabel);

    preferencesPanel.onAnalysisGainChanged = [this] (float gain)
    {
        performancePreferences.audio.analysisGain = gain;
        performancePreferences = sanitizePerformancePreferences (performancePreferences);
        audioInputAnalyzer.setAnalysisGain (performancePreferences.audio.analysisGain);
    };
    preferencesPanel.onMidiPreferencesChanged = [this] (MidiPreferences preferences)
    {
        applyMidiPreferences (std::move (preferences));
    };
    addChildComponent (preferencesPanel);

    dumpProofButton.setButtonText ("Dump Proof");
    dumpProofButton.onClick = [this] { dumpProof(); };
    addAndMakeVisible (dumpProofButton);

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
    addChildComponent (shaderEditor);

    preview.onStatusMessage = [safe = juce::Component::SafePointer<MainComponent> (this)] (juce::String incomingStatus)
    {
        juce::MessageManager::callAsync ([safe, statusMessage = std::move (incomingStatus)]
        {
            if (safe != nullptr)
                safe->setShaderStatus (statusMessage);
        });
    };
    preview.onSaveWorkRequested = [safe = juce::Component::SafePointer<MainComponent> (this)] (GraphSession& session)
    {
        if (safe == nullptr)
            return CommandResult { false, "main component is gone" };

        return safe->saveActiveWork (session);
    };
    preview.onPublishModuleRequested = [safe = juce::Component::SafePointer<MainComponent> (this)] (GraphSession& session,
                                                                                                   const std::string& sourceNodeId)
    {
        if (safe == nullptr)
            return CommandResult { false, "main component is gone" };

        return safe->publishSelectedModule (session, sourceNodeId);
    };
    addAndMakeVisible (preview);

    startAudioInput();
    audioInputAnalyzer.setAnalysisGain (preferencesPanel.getAnalysisGain());
    applyMidiPreferences (preferencesPanel.getMidiPreferences());
    startTimerHz (30);

    if (dumpProofOnStart)
    {
        juce::Timer::callAfterDelay (750, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpProof();
        });
    }

    if (dumpAudioProofOnStart)
    {
        juce::Timer::callAfterDelay (2500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpAudioProof();
        });
    }

    if (dumpC2StorageProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC2StorageProof();
        });
    }

    if (dumpC3SaveWorkProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC3SaveWorkProof();
        });
    }

    if (dumpC4AIWorkerSaveWorkProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC4AIWorkerSaveWorkProof();
        });
    }

    if (dumpC5ModulePublishProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC5ModulePublishProof();
        });
    }

    if (dumpC5AIWorkerModulePublishProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC5AIWorkerModulePublishProof();
        });
    }

    if (dumpC5VisibleModulePublishProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC5VisibleModulePublishProof();
        });
    }

    if (dumpC6AnalyzerFamilyProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC6AnalyzerFamilyProof();
        });
    }

    if (dumpC6AIRepairLoopProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpC6AIRepairLoopProof();
        });
    }

    if (dumpPVAttackDetectorProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpPVAttackDetectorProof();
        });
    }

    if (dumpPVDensityDetectorProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpPVDensityDetectorProof();
        });
    }

    if (dumpPVSilenceDetectorProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpPVSilenceDetectorProof();
        });
    }

    if (dumpPVSustainDetectorProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpPVSustainDetectorProof();
        });
    }

    if (dumpPVResidueDetectorProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpPVResidueDetectorProof();
        });
    }

    if (dumpPVAggregatePressureProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpPVAggregatePressureProof();
        });
    }

    if (dumpPVB1AnalyzerEnvironmentProofOnStart)
    {
        juce::Timer::callAfterDelay (500, [safe = juce::Component::SafePointer<MainComponent> (this)]
        {
            if (safe != nullptr)
                safe->dumpPVB1AnalyzerEnvironmentProof();
        });
    }

    setSize (1440, 860);
}

MainComponent::~MainComponent()
{
    stopTimer();
    audioDeviceManager.removeAudioCallback (&audioInputAnalyzer);
    midiOutput.reset();
    preview.onStatusMessage = nullptr;
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (13, 15, 20));
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced (12);
    auto header = area.removeFromTop (28);

    graphLabel.setBounds (header.removeFromLeft (260));
    dumpProofButton.setBounds (header.removeFromRight (112));
    header.removeFromRight (10);
    statusLabel.setBounds (header);

    area.removeFromTop (8);

    auto audioRow = area.removeFromTop (24);
    audioStatusLabel.setBounds (audioRow.removeFromLeft (270));
    rmsLabel.setBounds (audioRow.removeFromLeft (118));
    peakLabel.setBounds (audioRow.removeFromLeft (118));
    loudnessLabel.setBounds (audioRow.removeFromLeft (160));
    activeLabel.setBounds (audioRow.removeFromLeft (104));
    midiStatusLabel.setBounds (audioRow);

    area.removeFromTop (10);

    preview.setBounds (area);
}

void MainComponent::dumpProof()
{
    const auto directory = proofDumpDirectory();
    statusLabel.setText ("proof dump requested: " + directory.getFullPathName(), juce::dontSendNotification);
    preview.requestProofDump (directory, graph);
}

void MainComponent::dumpAudioProof()
{
    const auto directory = audioProofDumpDirectory();

    if (! directory.createDirectory())
    {
        statusLabel.setText ("audio proof failed: could not create " + directory.getFullPathName(),
                             juce::dontSendNotification);
        return;
    }

    const auto snapshot = audioInputAnalyzer.getSnapshot();
    const auto runtimeRegistry = loadAudioProofRuntimeRegistry();

    if (! runtimeRegistry.ok)
    {
        statusLabel.setText ("audio proof failed: " + juce::String (runtimeRegistry.error),
                             juce::dontSendNotification);
        return;
    }

    const auto runtimeInput = makeRuntimeSyntheticAudioInputFromAnalyzerSnapshot (snapshot, 64);
    const auto runtimeExecution = executeRuntimeRegistryWithSyntheticAudio (runtimeRegistry.registry, runtimeInput);

    if (! runtimeExecution.ok)
    {
        statusLabel.setText ("audio proof failed: " + juce::String (runtimeExecution.error),
                             juce::dontSendNotification);
        return;
    }

    const auto bridge = makeLoudnessRuntimeBridgeSnapshot (runtimeExecution.snapshot, snapshot);
    const auto json = juce::String()
        + "{\n"
        + "  \"sampleRate\": " + juce::String (audioInputAnalyzer.getSampleRate(), 0) + ",\n"
        + "  \"bufferSize\": " + juce::String (audioInputAnalyzer.getBufferSize()) + ",\n"
        + "  \"rms\": " + juce::String (snapshot.rms, 6) + ",\n"
        + "  \"peak\": " + juce::String (snapshot.peak, 6) + ",\n"
        + "  \"loudness\": " + juce::String (snapshot.loudness, 6) + ",\n"
        + "  \"gate\": " + juce::String (snapshot.gate, 6) + ",\n"
        + "  \"confidence\": " + juce::String (snapshot.confidence, 6) + ",\n"
        + "  \"active\": " + juce::String (snapshot.active ? "true" : "false") + ",\n"
        + "  \"analysisGain\": " + juce::String (performancePreferences.audio.analysisGain, 3) + ",\n"
        + "  \"midi\": {\n"
        + "    \"streamEnabled\": " + juce::String (performancePreferences.midi.streamEnabled ? "true" : "false") + ",\n"
        + "    \"mapModeEnabled\": " + juce::String (performancePreferences.midi.mapModeEnabled ? "true" : "false") + ",\n"
        + "    \"channel\": " + juce::String (performancePreferences.midi.channel) + ",\n"
        + "    \"loudnessCc\": " + juce::String (performancePreferences.midi.loudnessCc) + ",\n"
        + "    \"mapCc\": " + juce::String (performancePreferences.midi.mapCc) + ",\n"
        + "    \"outputName\": \"" + juce::String (performancePreferences.midi.outputName) + "\"\n"
        + "  },\n"
        + "  \"sampleCounter\": " + juce::String (static_cast<juce::int64> (snapshot.sampleCounter)) + "\n"
        + "}\n";

    const auto audioStatsFile = directory.getChildFile ("audio_stats.json");
    const auto loudnessCompoundFile = directory.getChildFile ("loudness_compound.json");
    const auto loudnessRuntimeExecutionFile = directory.getChildFile ("loudness_runtime_execution.json");
    const auto loudnessRuntimeBridgeFile = directory.getChildFile ("loudness_runtime_bridge.json");

    if (! audioStatsFile.replaceWithText (json, false, false, "\n"))
    {
        statusLabel.setText ("audio proof failed: could not write " + audioStatsFile.getFullPathName(),
                             juce::dontSendNotification);
        return;
    }

    if (! loudnessCompoundFile.replaceWithText (juce::String::fromUTF8 (makeCompoundPatchJson (makeLoudnessCompoundPatchSpec()).c_str()),
                                                false,
                                                false,
                                                "\n"))
    {
        statusLabel.setText ("audio proof failed: could not write " + loudnessCompoundFile.getFullPathName(),
                             juce::dontSendNotification);
        return;
    }

    const auto runtimeExecutionJson = makeRuntimeExecutionJson (runtimeExecution.snapshot);

    if (! loudnessRuntimeExecutionFile.replaceWithText (juce::String::fromUTF8 (runtimeExecutionJson.c_str()),
                                                        false,
                                                        false,
                                                        "\n"))
    {
        statusLabel.setText ("audio proof failed: could not write " + loudnessRuntimeExecutionFile.getFullPathName(),
                             juce::dontSendNotification);
        return;
    }

    const auto runtimeBridgeJson = makeLoudnessRuntimeBridgeJson (bridge);

    if (! loudnessRuntimeBridgeFile.replaceWithText (juce::String::fromUTF8 (runtimeBridgeJson.c_str()), false, false, "\n"))
    {
        statusLabel.setText ("audio proof failed: could not write " + loudnessRuntimeBridgeFile.getFullPathName(),
                             juce::dontSendNotification);
        return;
    }

    statusLabel.setText ("audio proof dumped: " + directory.getFullPathName(), juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC2StorageProof()
{
    const auto directory = c2StorageProofDumpDirectory();
    const auto reportFile = directory.getChildFile ("reload_report.json");
    const auto savedPatchFile = directory.getChildFile ("saved_main.patch.json");

    if (const auto error = createDirectoryIfMissing (directory); ! error.empty())
    {
        statusLabel.setText ("c2 storage proof failed: " + juce::String (error), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    std::string workManifestPath;
    PatchDocumentLoadResult loadedPatch;
    std::string lastError;

    for (const auto& candidate : repoCandidatePaths ("fixtures/storage/c2-compound-work/myworld.work.json"))
    {
        const auto loaded = loadMainPatchDocumentForWork (candidate);
        if (loaded.ok)
        {
            workManifestPath = candidate;
            loadedPatch = loaded;
            break;
        }

        lastError = loaded.error;
    }

    if (! loadedPatch.ok)
    {
        const auto report = makeC2StorageReportJson (false,
                                                     {},
                                                     savedPatchFile.getFullPathName().toStdString(),
                                                     {},
                                                     makeGraphSession (GraphContract {}),
                                                     false,
                                                     false,
                                                     false,
                                                     0.0,
                                                     0.0,
                                                     lastError.empty() ? "could not load C2 work fixture" : lastError);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c2 storage proof failed: " + juce::String (lastError), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    auto activeSession = makeGraphSession (loadedPatch.document.graph);
    const auto activeDocument = makePatchDocument (loadedPatch.document.id,
                                                  loadedPatch.document.title,
                                                  activeSession.graph);
    const auto saveResult = savePatchDocument (savedPatchFile.getFullPathName().toStdString(), activeDocument);

    if (! saveResult.ok)
    {
        const auto report = makeC2StorageReportJson (false,
                                                     workManifestPath,
                                                     savedPatchFile.getFullPathName().toStdString(),
                                                     saveResult.status,
                                                     activeSession,
                                                     false,
                                                     false,
                                                     false,
                                                     0.0,
                                                     0.0,
                                                     saveResult.error);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c2 storage proof failed: " + juce::String (saveResult.error), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    const auto reloadedPatch = loadPatchDocument (savedPatchFile.getFullPathName().toStdString());
    if (! reloadedPatch.ok)
    {
        const auto report = makeC2StorageReportJson (false,
                                                     workManifestPath,
                                                     savedPatchFile.getFullPathName().toStdString(),
                                                     saveResult.status,
                                                     activeSession,
                                                     false,
                                                     false,
                                                     false,
                                                     0.0,
                                                     0.0,
                                                     reloadedPatch.error);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c2 storage proof failed: " + juce::String (reloadedPatch.error), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    auto reloadedSession = makeGraphSession (reloadedPatch.document.graph);

    CompoundPatchLoadResult loadedCompound;
    for (const auto& candidate : repoCandidatePaths ("fixtures/compounds/loudness.compound.json"))
    {
        const auto loaded = loadCompoundPatchSpec (candidate);
        if (loaded.ok)
        {
            loadedCompound = loaded;
            break;
        }

        lastError = loaded.error;
    }

    if (! loadedCompound.ok)
    {
        const auto report = makeC2StorageReportJson (false,
                                                     workManifestPath,
                                                     savedPatchFile.getFullPathName().toStdString(),
                                                     saveResult.status,
                                                     reloadedSession,
                                                     false,
                                                     false,
                                                     false,
                                                     0.0,
                                                     0.0,
                                                     lastError.empty() ? "could not load loudness compound fixture" : lastError);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c2 storage proof failed: " + juce::String (lastError), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    const auto relayoutGraph = makeCompoundPatchInteractionGraph (loadedCompound.spec,
                                                                  "library_loud1",
                                                                  reloadedSession.graph);
    const auto* monoMix = findEditorNode (relayoutGraph, "library_loud1/mono_mix");
    const auto monoMixX = monoMix == nullptr ? 0.0 : monoMix->position.x;
    const auto monoMixY = monoMix == nullptr ? 0.0 : monoMix->position.y;
    const auto publicInputEdge = hasEdgeId (reloadedSession.graph, "edge.live_audio.channels.library_loud1.audio.in");
    const auto publicOutputEdge = hasEdgeId (reloadedSession.graph, "edge.library_loud1.out.midi_loudness.value");
    const auto monoMixLayout = monoMix != nullptr && monoMixX == 358.0 && monoMixY == 146.0;
    const auto graphCountsMatch = reloadedSession.graph.editorGraph.edges.size()
                                  == reloadedSession.graph.runtimeGraph.edges.size();
    const auto ok = publicInputEdge && publicOutputEdge && monoMixLayout && graphCountsMatch;

    const auto report = makeC2StorageReportJson (ok,
                                                 workManifestPath,
                                                 savedPatchFile.getFullPathName().toStdString(),
                                                 saveResult.status,
                                                 reloadedSession,
                                                 publicInputEdge,
                                                 publicOutputEdge,
                                                 monoMixLayout,
                                                 monoMixX,
                                                 monoMixY,
                                                 ok ? std::string {} : "reloaded C2 graph did not match expected compound work");

    if (! writeTextFile (reportFile, report))
    {
        statusLabel.setText ("c2 storage proof failed: could not write " + reportFile.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    statusLabel.setText ((ok ? "c2 storage proof dumped: " : "c2 storage proof mismatch: ")
                             + directory.getFullPathName(),
                         juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC3SaveWorkProof()
{
    const auto directory = c3SaveWorkProofDumpDirectory();
    const auto workDirectory = directory.getChildFile ("work");
    const auto patchDirectory = workDirectory.getChildFile ("patches");
    const auto reportFile = directory.getChildFile ("save_work_report.json");
    const auto workManifestFile = workDirectory.getChildFile ("myworld.work.json");
    const auto savedPatchFile = patchDirectory.getChildFile ("main.patch.json");

    if (const auto error = clearDirectoryIfExists (directory); ! error.empty())
    {
        statusLabel.setText ("c3 save_work proof failed: " + juce::String (error), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    if (const auto error = createDirectoryIfMissing (patchDirectory); ! error.empty())
    {
        statusLabel.setText ("c3 save_work proof failed: " + juce::String (error), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    std::string lastError;
    bool copiedFixture = false;
    for (const auto& candidate : repoCandidatePaths ("fixtures/storage/c2-compound-work/myworld.work.json"))
    {
        const auto sourceManifest = juce::File (candidate);
        const auto sourcePatch = sourceManifest.getParentDirectory().getChildFile ("patches").getChildFile ("main.patch.json");
        copiedFixture = copyTextFile (sourceManifest, workManifestFile) && copyTextFile (sourcePatch, savedPatchFile);
        if (copiedFixture)
            break;

        lastError = "could not copy C2 work fixture from " + candidate;
    }

    if (! copiedFixture)
    {
        const SaveLogLoadResult emptySaveLog;
        const auto report = makeC3SaveWorkReportJson (false,
                                                      workManifestFile.getFullPathName().toStdString(),
                                                      savedPatchFile.getFullPathName().toStdString(),
                                                      {},
                                                      {},
                                                      {},
                                                      emptySaveLog,
                                                      makeGraphSession (GraphContract {}),
                                                      false,
                                                      false,
                                                      false,
                                                      0.0,
                                                      0.0,
                                                      lastError.empty() ? "could not copy C3 work fixture" : lastError);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c3 save_work proof failed: " + juce::String (lastError), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    const auto loadedPatch = loadMainPatchDocumentForWork (workManifestFile.getFullPathName().toStdString());
    if (! loadedPatch.ok)
    {
        const SaveLogLoadResult emptySaveLog;
        const auto report = makeC3SaveWorkReportJson (false,
                                                      workManifestFile.getFullPathName().toStdString(),
                                                      savedPatchFile.getFullPathName().toStdString(),
                                                      {},
                                                      {},
                                                      {},
                                                      emptySaveLog,
                                                      makeGraphSession (GraphContract {}),
                                                      false,
                                                      false,
                                                      false,
                                                      0.0,
                                                      0.0,
                                                      loadedPatch.error);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c3 save_work proof failed: " + juce::String (loadedPatch.error), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    auto activeSession = makeGraphSession (loadedPatch.document.graph);
    const auto moveResult = moveNode (activeSession, "library_loud1", 13.0, 7.0);
    if (! moveResult.ok)
    {
        const SaveLogLoadResult emptySaveLog;
        const auto report = makeC3SaveWorkReportJson (false,
                                                      workManifestFile.getFullPathName().toStdString(),
                                                      savedPatchFile.getFullPathName().toStdString(),
                                                      {},
                                                      {},
                                                      {},
                                                      emptySaveLog,
                                                      activeSession,
                                                      false,
                                                      false,
                                                      false,
                                                      0.0,
                                                      0.0,
                                                      moveResult.message);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c3 save_work proof failed: " + juce::String (moveResult.message), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    const auto saveResult = saveWork (activeSession, workManifestFile.getFullPathName().toStdString());
    const auto reloadedPatch = loadPatchDocument (savedPatchFile.getFullPathName().toStdString());
    const auto saveLog = loadSaveLog (saveResult.saveLogPath);
    auto reloadedSession = reloadedPatch.ok ? makeGraphSession (reloadedPatch.document.graph) : makeGraphSession (GraphContract {});

    CompoundPatchLoadResult loadedCompound;
    for (const auto& candidate : repoCandidatePaths ("fixtures/compounds/loudness.compound.json"))
    {
        const auto loaded = loadCompoundPatchSpec (candidate);
        if (loaded.ok)
        {
            loadedCompound = loaded;
            break;
        }

        lastError = loaded.error;
    }

    const auto relayoutGraph = loadedCompound.ok
        ? makeCompoundPatchInteractionGraph (loadedCompound.spec, "library_loud1", reloadedSession.graph)
        : GraphContract {};
    const auto* monoMix = findEditorNode (relayoutGraph, "library_loud1/mono_mix");
    const auto monoMixX = monoMix == nullptr ? 0.0 : monoMix->position.x;
    const auto monoMixY = monoMix == nullptr ? 0.0 : monoMix->position.y;
    const auto publicInputEdge = hasEdgeId (reloadedSession.graph, "edge.live_audio.channels.library_loud1.audio.in");
    const auto publicOutputEdge = hasEdgeId (reloadedSession.graph, "edge.library_loud1.out.midi_loudness.value");
    const auto monoMixLayout = monoMix != nullptr && monoMixX == 358.0 && monoMixY == 146.0;
    const auto graphCountsMatch = reloadedSession.graph.editorGraph.edges.size()
                                  == reloadedSession.graph.runtimeGraph.edges.size();
    const auto commandLogStatus = activeSession.commandLog.empty() ? std::string {} : activeSession.commandLog.back();
    const auto saveLogStatus = saveLog.entries.empty() ? std::string {} : saveLog.entries.back().status;
    const auto saveLogCommitStatus = saveLog.entries.empty() ? std::string {} : saveLog.entries.back().commitStatus;
    const auto ok = saveResult.ok
                    && reloadedPatch.ok
                    && saveLog.ok
                    && commandLogStatus == "save_work:save-ok commit-pending"
                    && saveLogStatus == "save-ok commit-pending"
                    && saveLogCommitStatus == "not-started"
                    && publicInputEdge
                    && publicOutputEdge
                    && monoMixLayout
                    && graphCountsMatch;
    const auto error = ok ? std::string {}
                          : ! saveResult.ok ? saveResult.error
                          : ! reloadedPatch.ok ? reloadedPatch.error
                          : ! saveLog.ok ? saveLog.error
                          : ! loadedCompound.ok ? lastError
                          : "C3 save_work proof did not match expected command/storage evidence";

    const auto report = makeC3SaveWorkReportJson (ok,
                                                  workManifestFile.getFullPathName().toStdString(),
                                                  savedPatchFile.getFullPathName().toStdString(),
                                                  saveResult.saveLogPath,
                                                  saveResult.status,
                                                  commandLogStatus,
                                                  saveLog,
                                                  reloadedSession,
                                                  publicInputEdge,
                                                  publicOutputEdge,
                                                  monoMixLayout,
                                                  monoMixX,
                                                  monoMixY,
                                                  error);

    if (! writeTextFile (reportFile, report))
    {
        statusLabel.setText ("c3 save_work proof failed: could not write " + reportFile.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    statusLabel.setText ((ok ? "c3 save_work proof dumped: " : "c3 save_work proof mismatch: ")
                             + directory.getFullPathName(),
                         juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC4AIWorkerSaveWorkProof()
{
    const auto directory = c4AIWorkerSaveWorkProofDumpDirectory();

    C4AIWorkerSaveWorkProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC4AIWorkerSaveWorkProof (request);
    const auto displayNameString = juce::String (c4AIWorkerSaveWorkProofDisplayName());

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC5ModulePublishProof()
{
    const auto directory = c5ModulePublishProofDumpDirectory();

    C5ModulePublishProofRunRequest request;
    request.kind = C5ModulePublishProofKind::modulePublish;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC5ModulePublishProof (request);
    const auto displayNameString = juce::String (c5ModulePublishProofDisplayName (request.kind));

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC5AIWorkerModulePublishProof()
{
    const auto directory = c5AIWorkerModulePublishProofDumpDirectory();

    C5ModulePublishProofRunRequest request;
    request.kind = C5ModulePublishProofKind::aiWorkerModulePublish;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC5ModulePublishProof (request);
    const auto displayNameString = juce::String (c5ModulePublishProofDisplayName (request.kind));

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC5VisibleModulePublishProof()
{
    const auto proofDirectory = c5VisibleModulePublishProofDumpDirectory();

    C5ModulePublishProofRunRequest request;
    request.kind = C5ModulePublishProofKind::visibleModulePublish;
    request.outputDirectory = proofDirectory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC5ModulePublishProof (request);
    const auto displayNameString = juce::String (c5ModulePublishProofDisplayName (request.kind));

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + proofDirectory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC6AnalyzerFamilyProof()
{
    const auto directory = c6AnalyzerFamilyProofDumpDirectory();

    C6AnalyzerFamilyProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC6AnalyzerFamilyProof (request);
    const auto displayNameString = juce::String (c6AnalyzerFamilyProofDisplayName());

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC6AIRepairLoopProof()
{
    const auto directory = c6AIRepairLoopProofDumpDirectory();

    C6AIRepairLoopProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runC6AIRepairLoopProof (request);
    const auto displayNameString = juce::String (c6AIRepairLoopProofDisplayName());

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpPVAttackDetectorProof()
{
    dumpPVDetectorProof (PVDetectorProofKind::attack);
}

void MainComponent::dumpPVDensityDetectorProof()
{
    dumpPVDetectorProof (PVDetectorProofKind::density);
}

void MainComponent::dumpPVSilenceDetectorProof()
{
    dumpPVDetectorProof (PVDetectorProofKind::silence);
}

void MainComponent::dumpPVSustainDetectorProof()
{
    dumpPVDetectorProof (PVDetectorProofKind::sustain);
}

void MainComponent::dumpPVResidueDetectorProof()
{
    dumpPVDetectorProof (PVDetectorProofKind::residue);
}

void MainComponent::dumpPVAggregatePressureProof()
{
    dumpPVDetectorProof (PVDetectorProofKind::aggregatePressure);
}

void MainComponent::dumpPVDetectorProof (PVDetectorProofKind kind)
{
    const auto directory = pvDetectorProofDumpDirectory (kind);

    PVDetectorProofRunRequest request;
    request.kind = kind;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runPVDetectorProof (request);
    const auto displayName = juce::String (pvDetectorProofDisplayName (kind));

    if (result.status == "failed")
    {
        statusLabel.setText (displayName + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    }
    else
    {
        statusLabel.setText (displayName + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);
    }

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpPVB1AnalyzerEnvironmentProof()
{
    const auto directory = pvB1AnalyzerEnvironmentProofDumpDirectory();

    PVB1AnalyzerEnvironmentProofRunRequest request;
    request.outputDirectory = directory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();

    const auto result = runPVB1AnalyzerEnvironmentProof (request);
    const auto displayNameString = juce::String (pvB1AnalyzerEnvironmentProofDisplayName());

    if (result.status == "failed")
        statusLabel.setText (displayNameString + " proof failed: " + juce::String (result.error),
                             juce::dontSendNotification);
    else
        statusLabel.setText (displayNameString + " proof " + juce::String (result.status) + ": "
                                 + directory.getFullPathName(),
                             juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

CommandResult MainComponent::saveActiveWork (GraphSession& session)
{
    const auto manifestFile = activeWorkManifestFile();
    std::string error;

    if (manifestFile == defaultActiveWorkManifestFile()
        && ! prepareDefaultActiveWorkProject (manifestFile, error))
    {
        statusLabel.setText ("save_work failed: " + juce::String (error), juce::dontSendNotification);
        return { false, error };
    }

    const auto result = saveWork (session, manifestFile.getFullPathName().toStdString());
    const auto message = result.ok ? result.status : result.error;
    statusLabel.setText ((result.ok ? "save_work: " : "save_work failed: ") + juce::String (message),
                         juce::dontSendNotification);

    return { result.ok, message };
}

PublishModuleResult MainComponent::publishSelectedModuleResult (GraphSession& session, const std::string& sourceNodeId)
{
    const auto manifestFile = activeWorkManifestFile();
    std::string error;
    std::string workManifestPath;

    if (manifestFile == defaultActiveWorkManifestFile())
    {
        for (const auto& candidate : repoCandidatePaths ("fixtures/storage/c2-compound-work/myworld.work.json"))
        {
            const auto work = loadWorkProjectManifest (candidate);
            if (work.ok)
            {
                workManifestPath = candidate;
                break;
            }

            error = work.error;
        }
    }
    else
    {
        workManifestPath = manifestFile.getFullPathName().toStdString();
    }

    if (workManifestPath.empty())
    {
        return { false,
                 "publish_module",
                 "validation-failed",
                 sourceNodeId,
                 {},
                 {},
                 {},
                 {},
                 {},
                 {},
                 error.empty() ? "could not resolve publish source work manifest" : error };
    }

    const auto safeNodeId = safeIdentifier (sourceNodeId);
    const auto publishDirectory = c5VisibleModulePublishDirectory();

    PublishModuleRequest request;
    request.workManifestPath = workManifestPath;
    request.sourceNodeId = sourceNodeId;
    request.moduleId = "module.visible-" + safeNodeId;
    request.moduleTitle = "Visible " + sourceNodeId;
    request.nodeType = "compound.visible-" + safeNodeId;
    request.packageDirectory = publishDirectory.getChildFile ("modules")
                                   .getChildFile (safeNodeId)
                                   .getFullPathName()
                                   .toStdString();
    request.targetLibraryPath = publishDirectory.getChildFile ("module-libraries")
                                    .getChildFile ("visible.module-library.json")
                                    .getFullPathName()
                                    .toStdString();
    request.overwriteExisting = true;

    return publishModule (session, request);
}

CommandResult MainComponent::publishSelectedModule (GraphSession& session, const std::string& sourceNodeId)
{
    const auto result = publishSelectedModuleResult (session, sourceNodeId);
    const auto message = result.ok ? result.status : result.error;
    statusLabel.setText ((result.ok ? "publish_module: " : "publish_module failed: ") + juce::String (message),
                         juce::dontSendNotification);

    return { result.ok, message };
}

void MainComponent::timerCallback()
{
    updateAudioMeters();
}

void MainComponent::quitAfterDelay()
{
    juce::Timer::callAfterDelay (250, []
    {
        if (auto* app = juce::JUCEApplicationBase::getInstance())
            app->systemRequestedQuit();
    });
}

void MainComponent::setShaderStatus (juce::String message)
{
    const auto shouldQuit = shouldQuitAfterStartupDump && message.startsWith ("proof dumped:");
    statusLabel.setText (std::move (message), juce::dontSendNotification);

    if (shouldQuit)
        quitAfterDelay();
}

void MainComponent::startAudioInput()
{
    const auto error = audioDeviceManager.initialiseWithDefaultDevices (1, 0);

    if (error.isNotEmpty())
    {
        audioStatusLabel.setText ("audio input error: " + error, juce::dontSendNotification);
        return;
    }

    audioDeviceManager.addAudioCallback (&audioInputAnalyzer);
    audioStatusLabel.setText ("audio input ready", juce::dontSendNotification);
}

void MainComponent::updateAudioMeters()
{
    const auto bridge = makeLoudnessRuntimeBridgeSnapshot (RuntimeExecutionSnapshot{}, audioInputAnalyzer.getSnapshot());
    const auto& snapshot = bridge.analyzer;

    rmsLabel.setText ("rms " + juce::String (snapshot.rms, 4), juce::dontSendNotification);
    peakLabel.setText ("peak " + juce::String (snapshot.peak, 4), juce::dontSendNotification);
    loudnessLabel.setText ("loudness " + juce::String (snapshot.loudness, 4), juce::dontSendNotification);
    activeLabel.setText (juce::String ("active ") + (snapshot.active ? "yes" : "no"), juce::dontSendNotification);
    preview.setLoudness (snapshot.loudness);
    sendMidiForSnapshot (snapshot);
    midiStatusLabel.setText (midiStatus, juce::dontSendNotification);

    const auto sampleRate = audioInputAnalyzer.getSampleRate();

    if (sampleRate > 0.0)
    {
        audioStatusLabel.setText ("audio input "
                                      + juce::String (sampleRate, 0)
                                      + "Hz / "
                                      + juce::String (audioInputAnalyzer.getBufferSize())
                                      + " samples",
                                  juce::dontSendNotification);
    }
}

void MainComponent::applyMidiPreferences (MidiPreferences preferences)
{
    performancePreferences.midi = std::move (preferences);
    performancePreferences = sanitizePerformancePreferences (performancePreferences);

    const auto identifier = juce::String (performancePreferences.midi.outputIdentifier);

    if (identifier == openedMidiOutputIdentifier)
        return;

    midiOutput.reset();
    openedMidiOutputIdentifier = identifier;

    if (identifier.isEmpty())
    {
        midiStatus = "midi off";
        return;
    }

    midiOutput = juce::MidiOutput::openDevice (identifier);

    if (midiOutput == nullptr)
    {
        midiStatus = "midi open failed";
        return;
    }

    midiStatus = "midi " + juce::String (performancePreferences.midi.outputName);
}

void MainComponent::sendMidiForSnapshot (const AudioAnalyzerSnapshot& snapshot)
{
    const auto frame = makeLoudnessMidiCcFrame (snapshot.loudness, performancePreferences);

    if (! frame.shouldSend)
        return;

    if (midiOutput == nullptr)
    {
        midiStatus = "midi no output";
        return;
    }

    midiOutput->sendMessageNow (juce::MidiMessage::controllerEvent (frame.channel, frame.cc, frame.value));
    midiStatus = "midi ch" + juce::String (frame.channel)
                 + " cc" + juce::String (frame.cc)
                 + " " + juce::String (frame.value);
}
}
