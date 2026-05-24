#include "MainComponent.h"

#include "AIWorkerCommand.h"
#include "CompoundModule.h"
#include "CompoundPatch.h"
#include "GraphEndpoint.h"
#include "GraphContract.h"
#include "InteractionContract.h"
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
    return projectDirectory().getChildFile ("debug").getChildFile ("c4-ai-worker-save-work-proof");
}

juce::File c5ModulePublishProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c5-module-publish-proof");
}

juce::File c5AIWorkerModulePublishProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c5-ai-worker-module-publish-proof");
}

juce::File c5VisibleModulePublishDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c5-visible-module-publish");
}

juce::File c5VisibleModulePublishProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c5-visible-module-publish-proof");
}

juce::File c6AnalyzerFamilyProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c6-analyzer-family-proof");
}

juce::File c6AIRepairLoopProofDumpDirectory()
{
    return projectDirectory().getChildFile ("debug").getChildFile ("c6-ai-repair-loop-proof");
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

juce::String analyzerFamilyModuleLibraryPath()
{
    return "fixtures/module-libraries/analyzer-family.module-library.json";
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

const RuntimeOutputValue* findRuntimeOutput (const std::vector<RuntimeOutputValue>& outputs, const std::string& id)
{
    for (const auto& output : outputs)
        if (output.id == id)
            return &output;

    return nullptr;
}

double runtimeOutputValueOrZero (const std::vector<RuntimeOutputValue>& outputs, const std::string& id)
{
    const auto* output = findRuntimeOutput (outputs, id);
    return output == nullptr ? 0.0 : output->value;
}

constexpr const char* c6RawEnergyNodeType = "compound.raw-energy";
constexpr const char* c6LoudnessNodeType = "compound.loudness";
constexpr const char* c6RawEnergyNodeId = "raw_energy1";
constexpr const char* c6RmsOutputId = "rms";
constexpr const char* c6PeakOutputId = "peak";
constexpr const char* c6SampleCountOutputId = "sampleCount";
constexpr const char* proofWorkerId = "ai-worker-proof";
constexpr const char* loudnessCompoundNodeId = "library_loud1";
constexpr const char* c6RepairWorkerId = proofWorkerId;
constexpr const char* c6RepairId = "c6.2-ai-repair-loop";
constexpr const char* c6RepairMissingNodeId = "missing_loudness";
constexpr const char* c6RepairTargetNodeId = loudnessCompoundNodeId;
constexpr double c6RepairDeltaX = 17.0;
constexpr double c6RepairDeltaY = 5.0;
constexpr double c6UnusedRepairDelta = 100.0;

AIWorkerCommandRequest makeAIWorkerMoveNodeRequest (std::string commandId,
                                                    std::string workerId,
                                                    std::string intent,
                                                    std::string nodeId,
                                                    double deltaX,
                                                    double deltaY)
{
    AIWorkerCommandRequest request;
    request.commandId = std::move (commandId);
    request.workerId = std::move (workerId);
    request.operation = "move_node";
    request.intent = std::move (intent);
    request.nodeId = std::move (nodeId);
    request.deltaX = deltaX;
    request.deltaY = deltaY;
    return request;
}

AIWorkerCommandRequest makeC4MoveNodeProofRequest()
{
    return makeAIWorkerMoveNodeRequest ("c4.3-move-node",
                                        proofWorkerId,
                                        "Move the loaded loudness compound through the shared interaction command path",
                                        loudnessCompoundNodeId,
                                        13.0,
                                        7.0);
}

AIWorkerCommandRequest makeC4SaveWorkProofRequest (std::string workManifestPath)
{
    AIWorkerCommandRequest request;
    request.commandId = "c4.3-save-work";
    request.workerId = proofWorkerId;
    request.operation = "save_work";
    request.intent = "Persist AI-mutated C2 compound work through the shared save_work command path";
    request.workManifestPath = std::move (workManifestPath);
    return request;
}

PublishModuleRequest makeC5ModulePublishProofRequest (std::string workManifestPath,
                                                      std::string packageDirectory,
                                                      std::string targetLibraryPath)
{
    PublishModuleRequest request;
    request.workManifestPath = std::move (workManifestPath);
    request.sourceNodeId = loudnessCompoundNodeId;
    request.moduleId = "module.published-loudness";
    request.moduleTitle = "Published Loudness";
    request.nodeType = "compound.published-loudness";
    request.packageDirectory = std::move (packageDirectory);
    request.targetLibraryPath = std::move (targetLibraryPath);
    request.overwriteExisting = true;
    return request;
}

AIWorkerCommandRequest makeC5AIWorkerModulePublishProofRequest (std::string packageDirectory,
                                                                std::string targetLibraryPath)
{
    AIWorkerCommandRequest request;
    request.commandId = "c5.2-publish-module";
    request.workerId = proofWorkerId;
    request.operation = "publish_module";
    request.intent = "Publish the loaded loudness compound through the shared publish_module command path";
    request.nodeId = loudnessCompoundNodeId;
    request.moduleId = "module.ai-published-loudness";
    request.moduleTitle = "AI Published Loudness";
    request.publishedNodeType = "compound.ai-published-loudness";
    request.packageDirectory = std::move (packageDirectory);
    request.targetLibraryPath = std::move (targetLibraryPath);
    request.overwriteExisting = true;
    return request;
}

AIWorkerRepairPlan makeC6AIRepairLoopProofPlan()
{
    const auto failedAttempt = makeAIWorkerMoveNodeRequest ("c6.2-move-missing-node",
                                                           c6RepairWorkerId,
                                                           "First repair attempt intentionally targets a missing node",
                                                           c6RepairMissingNodeId,
                                                           c6RepairDeltaX,
                                                           c6RepairDeltaY);
    const auto repairedAttempt = makeAIWorkerMoveNodeRequest ("c6.2-move-library-loudness",
                                                             c6RepairWorkerId,
                                                             "Second repair attempt targets the loaded loudness node",
                                                             c6RepairTargetNodeId,
                                                             c6RepairDeltaX,
                                                             c6RepairDeltaY);
    auto unusedAttempt = repairedAttempt;
    unusedAttempt.commandId = "c6.2-unused-attempt";
    unusedAttempt.deltaX = c6UnusedRepairDelta;
    unusedAttempt.deltaY = c6UnusedRepairDelta;

    AIWorkerRepairPlan repairPlan;
    repairPlan.repairId = c6RepairId;
    repairPlan.workerId = c6RepairWorkerId;
    repairPlan.intent = "Repair a failed move_node command by retrying through the shared AI command path";
    repairPlan.maxAttempts = 3;
    repairPlan.attempts = { failedAttempt, repairedAttempt, unusedAttempt };
    return repairPlan;
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
    const auto workDirectory = directory.getChildFile ("work");
    const auto patchDirectory = workDirectory.getChildFile ("patches");
    const auto reportFile = directory.getChildFile ("ai_worker_save_work_report.json");
    const auto workManifestFile = workDirectory.getChildFile ("myworld.work.json");
    const auto savedPatchFile = patchDirectory.getChildFile ("main.patch.json");

    const auto moveRequest = makeC4MoveNodeProofRequest();
    const auto saveRequest = makeC4SaveWorkProofRequest (workManifestFile.getFullPathName().toStdString());

    const auto allowedOperations = allowedAIWorkerOperations();
    const AIWorkerCommandResult emptyMoveResult;
    const AIWorkerCommandResult emptySaveResult;
    const SaveLogLoadResult emptySaveLog;

    const auto writeFailureReport = [&] (const std::string& message, const GraphSession& reportSession)
    {
        const auto report = makeC4AIWorkerSaveWorkReportJson (false,
                                                              moveRequest,
                                                              emptyMoveResult,
                                                              saveRequest,
                                                              emptySaveResult,
                                                              allowedOperations,
                                                              emptySaveLog,
                                                              reportSession,
                                                              false,
                                                              false,
                                                              false,
                                                              0.0,
                                                              0.0,
                                                              false,
                                                              0.0,
                                                              0.0,
                                                              {},
                                                              message);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c4 AI worker proof failed: " + juce::String (message), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
    };

    if (const auto error = clearDirectoryIfExists (directory); ! error.empty())
    {
        writeFailureReport (error, makeGraphSession (GraphContract {}));
        return;
    }

    if (const auto error = createDirectoryIfMissing (patchDirectory); ! error.empty())
    {
        writeFailureReport (error, makeGraphSession (GraphContract {}));
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
        writeFailureReport (lastError.empty() ? "could not copy C4 work fixture" : lastError,
                            makeGraphSession (GraphContract {}));
        return;
    }

    const auto loadedPatch = loadMainPatchDocumentForWork (workManifestFile.getFullPathName().toStdString());
    if (! loadedPatch.ok)
    {
        writeFailureReport (loadedPatch.error, makeGraphSession (GraphContract {}));
        return;
    }

    auto activeSession = makeGraphSession (loadedPatch.document.graph);
    const auto moveResult = executeAIWorkerCommand (activeSession, moveRequest);
    if (! moveResult.ok)
    {
        const auto report = makeC4AIWorkerSaveWorkReportJson (false,
                                                              moveRequest,
                                                              moveResult,
                                                              saveRequest,
                                                              emptySaveResult,
                                                              allowedOperations,
                                                              emptySaveLog,
                                                              activeSession,
                                                              false,
                                                              false,
                                                              false,
                                                              0.0,
                                                              0.0,
                                                              false,
                                                              0.0,
                                                              0.0,
                                                              activeSession.commandLog.empty() ? std::string {} : activeSession.commandLog.back(),
                                                              moveResult.error);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c4 AI worker proof failed: " + juce::String (moveResult.error), juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    const auto saveResult = executeAIWorkerCommand (activeSession, saveRequest);
    const auto reloadedPatch = loadPatchDocument (saveResult.evidence.patchPath);
    const auto saveLog = loadSaveLog (saveResult.evidence.saveLogPath);
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
    const auto* savedMovedNode = findEditorNode (reloadedSession.graph, loudnessCompoundNodeId);
    const auto* activeMovedNode = findEditorNode (activeSession.graph, loudnessCompoundNodeId);
    const auto savedMoveX = savedMovedNode == nullptr ? 0.0 : savedMovedNode->position.x;
    const auto savedMoveY = savedMovedNode == nullptr ? 0.0 : savedMovedNode->position.y;
    const auto savedMovePersisted = savedMovedNode != nullptr
                                    && activeMovedNode != nullptr
                                    && nearlyEqual (savedMovedNode->position.x, activeMovedNode->position.x)
                                    && nearlyEqual (savedMovedNode->position.y, activeMovedNode->position.y);
    const auto monoMixLayout = monoMix != nullptr && monoMixX == 358.0 && monoMixY == 146.0;
    const auto graphCountsMatch = reloadedSession.graph.editorGraph.edges.size()
                                  == reloadedSession.graph.runtimeGraph.edges.size();
    const auto aiCommandLogStatus = activeSession.commandLog.empty() ? std::string {} : activeSession.commandLog.back();
    const auto saveLogStatus = saveLog.entries.empty() ? std::string {} : saveLog.entries.back().status;
    const auto saveWorkAllowed = std::find (allowedOperations.begin(), allowedOperations.end(), "save_work")
                                 != allowedOperations.end();
    const auto moveNodeAllowed = std::find (allowedOperations.begin(), allowedOperations.end(), "move_node")
                                 != allowedOperations.end();
    const auto hasMoveProof = std::any_of (activeSession.collaborationLog.begin(),
                                           activeSession.collaborationLog.end(),
                                           [] (const auto& item) {
                                               return item.proofEvidence.find ("graphCommandLogStatus=move_node")
                                                      != std::string::npos;
                                           });
    const auto collaborationLogOk = activeSession.collaborationLog.size() >= 4
                                    && activeSession.collaborationLog.front().operation == "move_node"
                                    && activeSession.collaborationLog.front().status == "requested"
                                    && activeSession.collaborationLog.back().operation == "save_work"
                                    && activeSession.collaborationLog.back().status == "save-ok commit-pending"
                                    && hasMoveProof
                                    && activeSession.collaborationLog.back().proofEvidence.find ("patchReloaded=true")
                                        != std::string::npos
                                    && activeSession.collaborationLog.back().proofEvidence.find ("saveLogStatus=save-ok commit-pending")
                                        != std::string::npos;
    const auto ok = moveResult.ok
                    && moveResult.operation == "move_node"
                    && moveResult.status == "ok"
                    && moveResult.evidence.graphCommandLogStatus == "move_node"
                    && moveResult.evidence.graphMutationApplied
                    && saveResult.ok
                    && saveResult.operation == "save_work"
                    && saveResult.status == "save-ok commit-pending"
                    && saveResult.evidence.storageCommandLogStatus == "save_work:save-ok commit-pending"
                    && saveResult.evidence.saveLogStatus == "save-ok commit-pending"
                    && ! saveResult.evidence.usesInteractionState
                    && aiCommandLogStatus == "ai_worker:save_work:save-ok commit-pending"
                    && saveWorkAllowed
                    && moveNodeAllowed
                    && reloadedPatch.ok
                    && saveLog.ok
                    && saveLogStatus == "save-ok commit-pending"
                    && collaborationLogOk
                    && savedMovePersisted
                    && publicInputEdge
                    && publicOutputEdge
                    && monoMixLayout
                    && graphCountsMatch;
    const auto error = ok ? std::string {}
                          : ! moveResult.ok ? moveResult.error
                          : ! saveResult.ok ? saveResult.error
                          : ! reloadedPatch.ok ? reloadedPatch.error
                          : ! saveLog.ok ? saveLog.error
                          : ! loadedCompound.ok ? lastError
                          : "C4 AI worker save_work proof did not match expected command/collaboration evidence";

    const auto report = makeC4AIWorkerSaveWorkReportJson (ok,
                                                          moveRequest,
                                                          moveResult,
                                                          saveRequest,
                                                          saveResult,
                                                          allowedOperations,
                                                          saveLog,
                                                          activeSession,
                                                          publicInputEdge,
                                                          publicOutputEdge,
                                                          monoMixLayout,
                                                          monoMixX,
                                                          monoMixY,
                                                          savedMovePersisted,
                                                          savedMoveX,
                                                          savedMoveY,
                                                          aiCommandLogStatus,
                                                          error);

    if (! writeTextFile (reportFile, report))
    {
        statusLabel.setText ("c4 AI worker proof failed: could not write " + reportFile.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    statusLabel.setText ((ok ? "c4 AI worker proof dumped: " : "c4 AI worker proof mismatch: ")
                             + directory.getFullPathName(),
                         juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC5ModulePublishProof()
{
    const auto directory = c5ModulePublishProofDumpDirectory();
    const auto reportFile = directory.getChildFile ("module_publish_report.json");
    const auto packageDirectory = directory.getChildFile ("modules").getChildFile ("published-loudness");
    const auto libraryFile = directory.getChildFile ("module-libraries").getChildFile ("published.module-library.json");
    const PublishModuleResult emptyPublish;

    const auto writeFailureReport = [&] (const PublishModuleResult& publish,
                                         const std::string& message,
                                         const std::string& runtimeCoverageStatus = {})
    {
        const auto report = makeC5ModulePublishReportJson (false,
                                                           publish,
                                                           false,
                                                           false,
                                                           false,
                                                           false,
                                                           runtimeCoverageStatus,
                                                           false,
                                                           {},
                                                           message);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c5 module publish proof failed: " + juce::String (message),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
    };

    if (const auto error = clearDirectoryIfExists (directory); ! error.empty())
    {
        writeFailureReport (emptyPublish, error);
        return;
    }

    if (const auto error = createDirectoryIfMissing (directory); ! error.empty())
    {
        writeFailureReport (emptyPublish, error);
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
        writeFailureReport (emptyPublish, lastError.empty() ? "could not load C2 work fixture" : lastError);
        return;
    }

    auto sourceSession = makeGraphSession (loadedPatch.document.graph);
    const auto request = makeC5ModulePublishProofRequest (workManifestPath,
                                                          packageDirectory.getFullPathName().toStdString(),
                                                          libraryFile.getFullPathName().toStdString());

    const auto publish = publishModule (sourceSession, request);
    if (! publish.ok)
    {
        writeFailureReport (publish, publish.error);
        return;
    }

    const auto package = loadModulePackageManifest (publish.moduleManifestPath);
    const auto library = loadModuleLibraryManifest (publish.targetLibraryPath);
    const auto loadedSpecs = loadCompoundModuleNodeSpecsFromLibrary (publish.targetLibraryPath);
    const auto runtime = loadRuntimeRegistryFromModuleLibrary (publish.targetLibraryPath);
    const auto coverage = runtime.ok ? inspectRuntimeOpCoverage (runtime.registry) : RuntimeOpCoverageResult {};
    const auto diagnostics = coverage.ok ? makeRuntimeOpModuleDiagnostics (coverage.snapshot)
                                         : std::vector<RuntimeOpModuleDiagnostic> {};

    const auto visibleRegistryContainsPublishedNode = loadedSpecs.ok
        && std::any_of (loadedSpecs.specs.begin(),
                        loadedSpecs.specs.end(),
                        [&publish] (const auto& spec) {
                            return spec.type == publish.publishedNodeType;
                        });
    const auto runtimeRegistryContainsPublishedNode = runtime.ok
        && std::any_of (runtime.registry.entries.begin(),
                        runtime.registry.entries.end(),
                        [&publish] (const auto& entry) {
                            return entry.nodeType == publish.publishedNodeType;
                        });
    const auto runtimeCoverageStatus = [&diagnostics, &publish]
    {
        for (const auto& diagnostic : diagnostics)
            if (diagnostic.nodeType == publish.publishedNodeType)
                return diagnostic.status == "runtime-op-ready" ? std::string { "ready" } : diagnostic.status;

        return std::string {};
    }();

    GraphSession reuseSession = makeGraphSession (makeDefaultShaderOutputGraph());
    CommandResult createResult { false, "published node spec not loaded" };
    if (loadedSpecs.ok)
    {
        const auto visibleRegistry = mergeNodeSpecs (makeSeedNodeSpecs(), loadedSpecs.specs);
        createResult = createNode (reuseSession,
                                   visibleRegistry,
                                   publish.publishedNodeType,
                                   "published_loud1",
                                   { 320.0, 260.0 });
    }

    const auto graphCommandLogStatus = reuseSession.commandLog.empty() ? std::string {}
                                                                       : reuseSession.commandLog.back();
    const auto createdPublishedNode = createResult.ok && graphCommandLogStatus == "create_node";
    const auto ok = publish.ok
                    && package.ok
                    && library.ok
                    && visibleRegistryContainsPublishedNode
                    && runtimeRegistryContainsPublishedNode
                    && runtimeCoverageStatus == "ready"
                    && createdPublishedNode;
    const auto error = ok ? std::string {}
                          : ! package.ok ? package.error
                          : ! library.ok ? library.error
                          : ! loadedSpecs.ok ? loadedSpecs.error
                          : ! runtime.ok ? runtime.error
                          : ! coverage.ok ? coverage.error
                          : ! createResult.ok ? createResult.message
                          : "C5 module publish proof did not match expected publish/reuse evidence";

    const auto report = makeC5ModulePublishReportJson (ok,
                                                       publish,
                                                       package.ok,
                                                       library.ok,
                                                       visibleRegistryContainsPublishedNode,
                                                       runtimeRegistryContainsPublishedNode,
                                                       runtimeCoverageStatus,
                                                       createdPublishedNode,
                                                       graphCommandLogStatus,
                                                       error);

    if (! writeTextFile (reportFile, report))
    {
        statusLabel.setText ("c5 module publish proof failed: could not write " + reportFile.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    statusLabel.setText ((ok ? "c5 module publish proof dumped: " : "c5 module publish proof mismatch: ")
                             + directory.getFullPathName(),
                         juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC5AIWorkerModulePublishProof()
{
    const auto directory = c5AIWorkerModulePublishProofDumpDirectory();
    const auto reportFile = directory.getChildFile ("ai_worker_module_publish_report.json");
    const auto packageDirectory = directory.getChildFile ("modules").getChildFile ("ai-published-loudness");
    const auto libraryFile = directory.getChildFile ("module-libraries").getChildFile ("ai-published.module-library.json");
    const auto allowedOperations = allowedAIWorkerOperations();
    const auto allowedPublishModule = std::find (allowedOperations.begin(),
                                                 allowedOperations.end(),
                                                 "publish_module") != allowedOperations.end();

    auto request = makeC5AIWorkerModulePublishProofRequest (packageDirectory.getFullPathName().toStdString(),
                                                            libraryFile.getFullPathName().toStdString());

    const AIWorkerCommandResult emptyResult;

    const auto writeFailureReport = [&] (const AIWorkerCommandResult& result, const std::string& message)
    {
        const auto report = makeC5AIWorkerModulePublishReportJson (false,
                                                                   allowedPublishModule,
                                                                   request,
                                                                   result,
                                                                   0,
                                                                   {},
                                                                   {},
                                                                   message);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c5 AI worker publish proof failed: " + juce::String (message),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
    };

    if (const auto error = clearDirectoryIfExists (directory); ! error.empty())
    {
        writeFailureReport (emptyResult, error);
        return;
    }

    if (const auto error = createDirectoryIfMissing (directory); ! error.empty())
    {
        writeFailureReport (emptyResult, error);
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
        writeFailureReport (emptyResult, lastError.empty() ? "could not load C2 work fixture" : lastError);
        return;
    }

    request.workManifestPath = workManifestPath;
    auto session = makeGraphSession (loadedPatch.document.graph);
    const auto result = executeAIWorkerCommand (session, request);
    const auto aiCommandLogStatus = session.commandLog.empty() ? std::string {} : session.commandLog.back();
    const auto collaborationProofEvidence = session.collaborationLog.empty() ? std::string {}
                                                                             : session.collaborationLog.back().proofEvidence;
    const auto collaborationLogOk = session.collaborationLog.size() >= 2
                                    && session.collaborationLog.front().operation == "publish_module"
                                    && session.collaborationLog.front().status == "requested"
                                    && session.collaborationLog.back().operation == "publish_module"
                                    && session.collaborationLog.back().status == "published"
                                    && collaborationProofEvidence.find ("publishCommandLogStatus=publish_module:published")
                                        != std::string::npos
                                    && collaborationProofEvidence.find ("packageReloaded=true") != std::string::npos
                                    && collaborationProofEvidence.find ("libraryReloaded=true") != std::string::npos;
    const auto ok = allowedPublishModule
                    && result.ok
                    && result.operation == "publish_module"
                    && result.status == "published"
                    && result.evidence.publishCommandLogStatus == "publish_module:published"
                    && result.evidence.packageReloaded
                    && result.evidence.libraryReloaded
                    && aiCommandLogStatus == "ai_worker:publish_module:published"
                    && collaborationLogOk;
    const auto error = ok ? std::string {}
                          : ! result.ok ? result.error
                          : "C5 AI worker publish_module proof did not match expected command/collaboration evidence";

    const auto report = makeC5AIWorkerModulePublishReportJson (ok,
                                                               allowedPublishModule,
                                                               request,
                                                               result,
                                                               session.collaborationLog.size(),
                                                               collaborationProofEvidence,
                                                               aiCommandLogStatus,
                                                               error);

    if (! writeTextFile (reportFile, report))
    {
        statusLabel.setText ("c5 AI worker publish proof failed: could not write " + reportFile.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    statusLabel.setText ((ok ? "c5 AI worker publish proof dumped: " : "c5 AI worker publish proof mismatch: ")
                             + directory.getFullPathName(),
                         juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC5VisibleModulePublishProof()
{
    const auto proofDirectory = c5VisibleModulePublishProofDumpDirectory();
    const auto publishDirectory = c5VisibleModulePublishDirectory();
    const auto reportFile = proofDirectory.getChildFile ("visible_module_publish_report.json");
    const PublishModuleResult emptyPublish;

    const auto writeFailureReport = [&] (const PublishModuleResult& publish, const std::string& message)
    {
        const auto report = makeC5VisibleModulePublishReportJson (false,
                                                                  publish,
                                                                  false,
                                                                  false,
                                                                  false,
                                                                  false,
                                                                  {},
                                                                  message);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c5 visible publish proof failed: " + juce::String (message),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
    };

    if (const auto error = clearDirectoryIfExists (proofDirectory); ! error.empty())
    {
        writeFailureReport (emptyPublish, error);
        return;
    }

    if (const auto error = clearDirectoryIfExists (publishDirectory); ! error.empty())
    {
        writeFailureReport (emptyPublish, error);
        return;
    }

    if (const auto error = createDirectoryIfMissing (proofDirectory); ! error.empty())
    {
        writeFailureReport (emptyPublish, error);
        return;
    }

    auto session = makeGraphSession (makeDefaultShaderOutputGraph());
    const auto createSource = createNode (session, "compound.loudness", "loud1", { 220.0, 260.0 });
    if (! createSource.ok)
    {
        writeFailureReport (emptyPublish, createSource.message);
        return;
    }

    const auto publish = publishSelectedModuleResult (session, "loud1");
    const auto package = publish.ok ? loadModulePackageManifest (publish.moduleManifestPath) : ModulePackageLoadResult {};
    const auto library = publish.ok ? loadModuleLibraryManifest (publish.targetLibraryPath) : ModuleLibraryLoadResult {};
    const auto loadedSpecs = publish.ok ? loadCompoundModuleNodeSpecsFromLibrary (publish.targetLibraryPath)
                                        : CompoundModuleNodeSpecsResult {};
    const auto packageReloaded = package.ok && std::filesystem::exists (publish.compoundPatchPath);
    const auto libraryReloaded = library.ok;
    const auto visibleRegistryContainsPublishedNode = loadedSpecs.ok
        && std::any_of (loadedSpecs.specs.begin(),
                        loadedSpecs.specs.end(),
                        [&publish] (const auto& spec) {
                            return spec.type == publish.publishedNodeType;
                        });

    auto reuseSession = makeGraphSession (makeDefaultShaderOutputGraph());
    CommandResult createResult { false, "published node spec not loaded" };
    if (loadedSpecs.ok)
    {
        const auto visibleRegistry = mergeNodeSpecs (makeSeedNodeSpecs(), loadedSpecs.specs);
        createResult = createNode (reuseSession,
                                   visibleRegistry,
                                   publish.publishedNodeType,
                                   "visible_published_loud1",
                                   { 340.0, 280.0 });
    }

    const auto graphCommandLogStatus = reuseSession.commandLog.empty() ? std::string {}
                                                                       : reuseSession.commandLog.back();
    const auto createdPublishedNode = createResult.ok && graphCommandLogStatus == "create_node";
    const auto ok = publish.ok
                    && publish.status == "published"
                    && packageReloaded
                    && libraryReloaded
                    && visibleRegistryContainsPublishedNode
                    && createdPublishedNode;
    const auto error = ok ? std::string {}
                          : ! publish.ok ? publish.error
                          : ! package.ok ? package.error
                          : ! library.ok ? library.error
                          : ! loadedSpecs.ok ? loadedSpecs.error
                          : ! createResult.ok ? createResult.message
                          : "C5 visible publish proof did not match expected publish/reuse evidence";

    const auto report = makeC5VisibleModulePublishReportJson (ok,
                                                              publish,
                                                              packageReloaded,
                                                              libraryReloaded,
                                                              visibleRegistryContainsPublishedNode,
                                                              createdPublishedNode,
                                                              graphCommandLogStatus,
                                                              error);

    if (! writeTextFile (reportFile, report))
    {
        statusLabel.setText ("c5 visible publish proof failed: could not write " + reportFile.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    statusLabel.setText ((ok ? "c5 visible publish proof dumped: " : "c5 visible publish proof mismatch: ")
                             + proofDirectory.getFullPathName(),
                         juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC6AnalyzerFamilyProof()
{
    const auto directory = c6AnalyzerFamilyProofDumpDirectory();
    const auto reportFile = directory.getChildFile ("analyzer_family_report.json");
    const auto libraryPath = analyzerFamilyModuleLibraryPath().toStdString();
    const std::vector<RuntimeOutputValue> emptyOutputs;

    const auto writeFailureReport = [&] (const std::string& message)
    {
        const auto report = makeC6AnalyzerFamilyReportJson (false,
                                                            libraryPath,
                                                            0,
                                                            false,
                                                            false,
                                                            {},
                                                            false,
                                                            {},
                                                            false,
                                                            emptyOutputs,
                                                            message);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c6 analyzer family proof failed: " + juce::String (message),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
    };

    if (const auto error = clearDirectoryIfExists (directory); ! error.empty())
    {
        writeFailureReport (error);
        return;
    }

    if (const auto error = createDirectoryIfMissing (directory); ! error.empty())
    {
        writeFailureReport (error);
        return;
    }

    const auto loadedSpecs = loadCompoundModuleNodeSpecsFromLibrary (libraryPath);
    const auto familyEntryCount = loadedSpecs.ok ? loadedSpecs.specs.size() : 0;
    const auto visibleRegistryContainsRawEnergy = loadedSpecs.ok
        && findNodeSpec (loadedSpecs.specs, c6RawEnergyNodeType) != nullptr;
    const auto loudnessStillPresent = loadedSpecs.ok
        && findNodeSpec (loadedSpecs.specs, c6LoudnessNodeType) != nullptr;

    const auto runtime = loadRuntimeRegistryFromModuleLibrary (libraryPath);
    const auto runtimeRegistryContainsRawEnergy = runtime.ok
        && std::any_of (runtime.registry.entries.begin(),
                        runtime.registry.entries.end(),
                        [] (const auto& entry) {
                            return entry.nodeType == c6RawEnergyNodeType;
                        });
    const auto coverage = runtime.ok ? inspectRuntimeOpCoverage (runtime.registry) : RuntimeOpCoverageResult {};
    const auto diagnostics = coverage.snapshot.entries.empty()
        ? std::vector<RuntimeOpModuleDiagnostic> {}
        : makeRuntimeOpModuleDiagnostics (coverage.snapshot);
    const auto runtimeCoverageStatus = [&diagnostics]
    {
        for (const auto& diagnostic : diagnostics)
        {
            if (diagnostic.nodeType != c6RawEnergyNodeType)
                continue;

            return diagnostic.status == "runtime-op-ready" ? std::string { "ready" } : diagnostic.status;
        }

        return std::string {};
    }();

    RuntimeSyntheticAudioInput input;
    input.channels = {
        { 0.0f, 1.0f, -1.0f, 0.0f },
        { 0.0f, 0.5f, -0.5f, 0.0f }
    };
    input.analysisGain = 1.5f;

    const auto execution = runtime.ok ? executeRuntimeRegistryWithSyntheticAudio (runtime.registry, input)
                                      : RuntimeExecutionResult {};
    std::vector<RuntimeOutputValue> rawEnergyPublicOutputs;
    std::string rawEnergyExecutionStatus;

    if (execution.ok)
    {
        for (const auto& entry : execution.snapshot.entries)
        {
            if (entry.nodeType != c6RawEnergyNodeType)
                continue;

            rawEnergyExecutionStatus = entry.status;
            rawEnergyPublicOutputs = entry.publicOutputs;
            break;
        }
    }

    const auto rawOutputsOk = rawEnergyExecutionStatus == "computed"
        && findRuntimeOutput (rawEnergyPublicOutputs, c6RmsOutputId) != nullptr
        && findRuntimeOutput (rawEnergyPublicOutputs, c6PeakOutputId) != nullptr
        && findRuntimeOutput (rawEnergyPublicOutputs, c6SampleCountOutputId) != nullptr
        && nearlyEqual (runtimeOutputValueOrZero (rawEnergyPublicOutputs, c6RmsOutputId), std::sqrt (0.28125))
        && nearlyEqual (runtimeOutputValueOrZero (rawEnergyPublicOutputs, c6PeakOutputId), 0.75)
        && nearlyEqual (runtimeOutputValueOrZero (rawEnergyPublicOutputs, c6SampleCountOutputId), 4.0);

    auto session = makeGraphSession (makeDefaultShaderOutputGraph());
    CommandResult createResult { false, "raw-energy node spec not loaded" };
    if (loadedSpecs.ok)
    {
        const auto visibleRegistry = mergeNodeSpecs (makeSeedNodeSpecs(), loadedSpecs.specs);
        createResult = createNode (session,
                                   visibleRegistry,
                                   c6RawEnergyNodeType,
                                   c6RawEnergyNodeId,
                                   { 300.0, 320.0 });
    }

    const auto graphCommandLogStatus = session.commandLog.empty() ? std::string {}
                                                                  : session.commandLog.back();
    const auto createdRawEnergyNode = createResult.ok && graphCommandLogStatus == "create_node";
    const auto ok = loadedSpecs.ok
                    && familyEntryCount == 2
                    && visibleRegistryContainsRawEnergy
                    && runtime.ok
                    && runtimeRegistryContainsRawEnergy
                    && coverage.ok
                    && runtimeCoverageStatus == "ready"
                    && execution.ok
                    && rawOutputsOk
                    && createdRawEnergyNode
                    && loudnessStillPresent;
    const auto error = ok ? std::string {}
                          : ! loadedSpecs.ok ? loadedSpecs.error
                          : ! runtime.ok ? runtime.error
                          : ! coverage.ok ? coverage.error
                          : ! execution.ok ? execution.error
                          : ! createResult.ok ? createResult.message
                          : "C6 analyzer family proof did not match expected raw-energy evidence";

    const auto report = makeC6AnalyzerFamilyReportJson (ok,
                                                        libraryPath,
                                                        familyEntryCount,
                                                        visibleRegistryContainsRawEnergy,
                                                        runtimeRegistryContainsRawEnergy,
                                                        runtimeCoverageStatus,
                                                        createdRawEnergyNode,
                                                        graphCommandLogStatus,
                                                        loudnessStillPresent,
                                                        rawEnergyPublicOutputs,
                                                        error);

    if (! writeTextFile (reportFile, report))
    {
        statusLabel.setText ("c6 analyzer family proof failed: could not write " + reportFile.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    statusLabel.setText ((ok ? "c6 analyzer family proof dumped: " : "c6 analyzer family proof mismatch: ")
                             + directory.getFullPathName(),
                         juce::dontSendNotification);

    if (shouldQuitAfterStartupDump)
        quitAfterDelay();
}

void MainComponent::dumpC6AIRepairLoopProof()
{
    const auto directory = c6AIRepairLoopProofDumpDirectory();
    const auto reportFile = directory.getChildFile ("ai_repair_loop_report.json");
    const AIWorkerRepairLoopResult emptyResult;

    const auto writeFailureReport = [&] (const AIWorkerRepairLoopResult& repairResult, const std::string& message)
    {
        const auto report = makeC6AIRepairLoopReportJson (false,
                                                          repairResult,
                                                          false,
                                                          0,
                                                          false,
                                                          0.0,
                                                          0.0,
                                                          message);
        writeTextFile (reportFile, report);
        statusLabel.setText ("c6 AI repair loop proof failed: " + juce::String (message),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
    };

    if (const auto error = clearDirectoryIfExists (directory); ! error.empty())
    {
        writeFailureReport (emptyResult, error);
        return;
    }

    if (const auto error = createDirectoryIfMissing (directory); ! error.empty())
    {
        writeFailureReport (emptyResult, error);
        return;
    }

    PatchDocumentLoadResult loadedPatch;
    std::string lastError;
    for (const auto& candidate : repoCandidatePaths ("fixtures/storage/c2-compound-work/myworld.work.json"))
    {
        const auto loaded = loadMainPatchDocumentForWork (candidate);
        if (loaded.ok)
        {
            loadedPatch = loaded;
            break;
        }

        lastError = loaded.error;
    }

    if (! loadedPatch.ok)
    {
        writeFailureReport (emptyResult, lastError.empty() ? "could not load C2 work fixture" : lastError);
        return;
    }

    auto session = makeGraphSession (loadedPatch.document.graph);

    const auto repairPlan = makeC6AIRepairLoopProofPlan();

    const auto repairResult = executeAIWorkerRepairLoop (session, repairPlan);
    const auto* finalNode = findEditorNode (session.graph, c6RepairTargetNodeId);
    const auto finalNodeX = finalNode == nullptr ? 0.0 : finalNode->position.x;
    const auto finalNodeY = finalNode == nullptr ? 0.0 : finalNode->position.y;
    const auto graphMutationApplied = ! repairResult.attempts.empty()
        && repairResult.attempts.back().commandResult.evidence.graphMutationApplied;
    const auto firstAttemptFailed = ! repairResult.attempts.empty()
        && repairResult.attempts.front().status == "failed";
    const auto ok = repairResult.ok
                    && repairResult.status == "repaired"
                    && repairResult.attemptsRun == 2
                    && repairResult.maxAttempts == 3
                    && firstAttemptFailed
                    && repairResult.successfulAttemptIndex == 2
                    && repairResult.finalOperation == "move_node"
                    && repairResult.finalCommandLogStatus == "ai_worker_repair_loop:repaired"
                    && graphMutationApplied
                    && session.collaborationLog.size() >= 6
                    && finalNode != nullptr;
    const auto error = ok ? std::string {}
                          : ! repairResult.ok ? repairResult.error
                          : "C6 AI repair loop proof did not match expected retry/repair evidence";

    const auto report = makeC6AIRepairLoopReportJson (ok,
                                                      repairResult,
                                                      graphMutationApplied,
                                                      session.collaborationLog.size(),
                                                      false,
                                                      finalNodeX,
                                                      finalNodeY,
                                                      error);

    if (! writeTextFile (reportFile, report))
    {
        statusLabel.setText ("c6 AI repair loop proof failed: could not write " + reportFile.getFullPathName(),
                             juce::dontSendNotification);
        if (shouldQuitAfterStartupDump)
            quitAfterDelay();
        return;
    }

    statusLabel.setText ((ok ? "c6 AI repair loop proof dumped: " : "c6 AI repair loop proof mismatch: ")
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
