#include "OpenGLShaderPreview.h"

#include "CompoundModule.h"
#include "RuntimeRegistry.h"

#include <algorithm>
#include <atomic>
#include <iterator>
#include <vector>

namespace myworld
{
namespace
{
bool writeTextFile (const juce::File& file, const std::string& text)
{
    return file.replaceWithText (juce::String::fromUTF8 (text.data(), static_cast<int> (text.size())),
                                 false,
                                 false,
                                 "\n");
}

bool writePngFile (const juce::File& file, const juce::Image& image)
{
    if (file.existsAsFile() && ! file.deleteFile())
        return false;

    auto output = file.createOutputStream();

    if (output == nullptr)
        return false;

    juce::PNGImageFormat pngFormat;
    return pngFormat.writeImageToStream (image, *output);
}

int mouseButtonIndex (const juce::MouseEvent& event)
{
    if (event.mods.isRightButtonDown())
        return 1;

    if (event.mods.isMiddleButtonDown())
        return 2;

    return 0;
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

juce::String missingRuntimeOpModuleLibraryPath()
{
    return "fixtures/module-libraries/missing-runtimeop.module-library.json";
}

juce::String pvAnalyzerVisibleModuleLibraryPath()
{
    return "fixtures/module-libraries/pv-analyzer-visible.module-library.json";
}

std::vector<juce::String> visibleModuleLibraryPaths()
{
    return {
        defaultModuleLibraryPath(),
        pvAnalyzerVisibleModuleLibraryPath()
    };
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

RuntimeRegistryLoadResult loadRuntimeRegistryFromCandidates (const juce::String& libraryPath)
{
    std::string lastError;

    for (const auto& path : moduleLibraryCandidatePaths (libraryPath))
    {
        const auto registry = loadRuntimeRegistryFromModuleLibrary (path);
        if (registry.ok)
            return registry;

        lastError = registry.error;
    }

    return { false, {}, lastError.empty() ? "could not load module library: " + libraryPath.toStdString()
                                          : lastError };
}

void appendRuntimeOpModuleDiagnostics (std::vector<RuntimeOpModuleDiagnostic>& diagnostics,
                                       const RuntimeOpCoverageResult& coverage)
{
    if (coverage.snapshot.entries.empty())
        return;

    auto nextDiagnostics = makeRuntimeOpModuleDiagnostics (coverage.snapshot);
    for (auto& diagnostic : nextDiagnostics)
    {
        const auto alreadyPresent = std::any_of (diagnostics.begin(),
                                                 diagnostics.end(),
                                                 [&diagnostic] (const auto& existing) {
                                                     return existing.nodeType == diagnostic.nodeType;
                                                 });
        if (! alreadyPresent)
            diagnostics.push_back (std::move (diagnostic));
    }
}

std::vector<RuntimeOpModuleDiagnostic> loadRuntimeOpModuleDiagnostics()
{
    std::vector<RuntimeOpModuleDiagnostic> diagnostics;

    for (const auto& libraryPath : visibleModuleLibraryPaths())
    {
        const auto runtimeRegistry = loadRuntimeRegistryFromCandidates (libraryPath);
        if (runtimeRegistry.ok)
            appendRuntimeOpModuleDiagnostics (diagnostics, inspectRuntimeOpCoverage (runtimeRegistry.registry));
    }

    const auto missingRuntimeOpRegistry = loadRuntimeRegistryFromCandidates (missingRuntimeOpModuleLibraryPath());
    if (missingRuntimeOpRegistry.ok)
        appendRuntimeOpModuleDiagnostics (diagnostics, inspectRuntimeOpCoverage (missingRuntimeOpRegistry.registry));

    return diagnostics;
}

std::vector<NodeSpec> loadVisibleNodeSpecs()
{
    auto visibleSpecs = makeSeedNodeSpecs();

    for (const auto& libraryPath : visibleModuleLibraryPaths())
    {
        for (const auto& path : moduleLibraryCandidatePaths (libraryPath))
        {
            const auto modules = loadCompoundModuleNodeSpecsFromLibrary (path);
            if (modules.ok)
            {
                visibleSpecs = mergeNodeSpecs (std::move (visibleSpecs), modules.specs);
                break;
            }
        }
    }

    return visibleSpecs;
}

RuntimeRegistry loadVisibleRuntimeRegistry()
{
    return loadRuntimeRegistryFromCandidates (defaultModuleLibraryPath()).registry;
}
}

OpenGLShaderPreview::OpenGLShaderPreview()
    : renderBackend (openGLContext),
      pendingFragmentShader (defaultFragmentShader())
{
    setWantsKeyboardFocus (true);

    imguiOverlay.onShaderSourceSubmitted = [this] (const std::string& source)
    {
        setFragmentShader (source);
    };

    imguiOverlay.onSaveWorkRequested = [this] (GraphSession& session)
    {
        if (onSaveWorkRequested == nullptr)
            return CommandResult { false, "no active work" };

        return onSaveWorkRequested (session);
    };

    imguiOverlay.onPublishModuleRequested = [this] (GraphSession& session, const std::string& sourceNodeId)
    {
        if (onPublishModuleRequested == nullptr)
            return CommandResult { false, "no active publisher" };

        return onPublishModuleRequested (session, sourceNodeId);
    };

    openGLContext.setOpenGLVersionRequired (juce::OpenGLContext::openGL3_2);
    openGLContext.setRenderer (this);
    openGLContext.setContinuousRepainting (false);
    openGLContext.attachTo (*this);

    startTimerHz (60);
}

OpenGLShaderPreview::~OpenGLShaderPreview()
{
    stopTimer();
    onStatusMessage = nullptr;
    openGLContext.detach();
}

void OpenGLShaderPreview::setFragmentShader (std::string source)
{
    const juce::ScopedLock lock (shaderLock);
    pendingFragmentShader = std::move (source);
    compileRequested = true;
    openGLContext.triggerRepaint();
}

void OpenGLShaderPreview::setLoudness (float newLoudness)
{
    loudness.store (juce::jlimit (0.0f, 1.0f, newLoudness), std::memory_order_relaxed);
}

void OpenGLShaderPreview::requestProofDump (juce::File outputDirectory, GraphContract graph)
{
    {
        const juce::ScopedLock lock (proofDumpLock);
        pendingProofDump = std::make_unique<PendingProofDump> (PendingProofDump { std::move (outputDirectory),
                                                                                  std::move (graph) });
    }

    openGLContext.triggerRepaint();
}

void OpenGLShaderPreview::newOpenGLContextCreated()
{
    renderBackend.initialise();

    startTimeSeconds = juce::Time::getMillisecondCounterHiRes() * 0.001;
    lastFrameSeconds = startTimeSeconds;
    frameIndex = 0;
    seedNodeSpecs = loadVisibleNodeSpecs();
    runtimeOpDiagnostics = loadRuntimeOpModuleDiagnostics();
    imguiOverlay.setRuntimeOpDiagnostics (runtimeOpDiagnostics);
    loudnessCompound = makeLoudnessCompoundPatchSpec();
    imguiOverlay.setShaderSource (pendingFragmentShader);
    imguiOverlay.initialise();

    compilePendingShader();
}

void OpenGLShaderPreview::renderOpenGL()
{
    using namespace ::juce::gl;

    jassert (juce::OpenGLHelpers::isContextActive());

    compilePendingShader();

    const auto scale = static_cast<float> (openGLContext.getRenderingScale());
    const auto width = juce::roundToInt (static_cast<float> (getWidth()) * scale);
    const auto height = juce::roundToInt (static_cast<float> (getHeight()) * scale);

    const auto nowSeconds = juce::Time::getMillisecondCounterHiRes() * 0.001;
    const auto elapsed = static_cast<float> (nowSeconds - startTimeSeconds);
    renderBackend.resize (juce::jmax (1, width), juce::jmax (1, height), scale);
    renderBackend.renderFrame ({ elapsed, frameIndex, loudness.load (std::memory_order_relaxed) });

    const auto deltaSeconds = static_cast<float> (nowSeconds - lastFrameSeconds);
    lastFrameSeconds = nowSeconds;
    imguiOverlay.beginFrame (juce::jmax (1, width), juce::jmax (1, height), scale, deltaSeconds);
    imguiOverlay.drawSmokePanel (seedNodeSpecs, loudnessCompound, lastStatus.toStdString());
    imguiOverlay.render();

    handlePendingProofDump (juce::jmax (1, width), juce::jmax (1, height), elapsed, frameIndex);
    ++frameIndex;
}

void OpenGLShaderPreview::openGLContextClosing()
{
    releaseGLObjects();
}

void OpenGLShaderPreview::timerCallback()
{
    openGLContext.triggerRepaint();
}

void OpenGLShaderPreview::mouseMove (const juce::MouseEvent& event)
{
    updateImGuiMousePosition (event);
}

void OpenGLShaderPreview::mouseDown (const juce::MouseEvent& event)
{
    grabKeyboardFocus();
    updateImGuiMousePosition (event);
    imguiOverlay.setMouseButton (mouseButtonIndex (event), true);
}

void OpenGLShaderPreview::mouseDrag (const juce::MouseEvent& event)
{
    updateImGuiMousePosition (event);
}

void OpenGLShaderPreview::mouseUp (const juce::MouseEvent& event)
{
    updateImGuiMousePosition (event);
    imguiOverlay.setMouseButton (0, false);
    imguiOverlay.setMouseButton (1, false);
    imguiOverlay.setMouseButton (2, false);
}

void OpenGLShaderPreview::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    updateImGuiMousePosition (event);
    imguiOverlay.addMouseWheel (wheel.deltaY);
}

bool OpenGLShaderPreview::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey)
    {
        imguiOverlay.requestDeleteSelection();
        openGLContext.triggerRepaint();
        return true;
    }

    if (key == juce::KeyPress ('s', juce::ModifierKeys::commandModifier, 0))
    {
        imguiOverlay.requestSaveWork();
        openGLContext.triggerRepaint();
        return true;
    }

    return false;
}

void OpenGLShaderPreview::compilePendingShader()
{
    std::string source;

    {
        const juce::ScopedLock lock (shaderLock);

        if (! compileRequested)
            return;

        source = pendingFragmentShader;
        compileRequested = false;
    }

    const auto result = renderBackend.compileShader (source);
    reportStatus (result.message);
}

void OpenGLShaderPreview::handlePendingProofDump (int width,
                                                  int height,
                                                  double timeSeconds,
                                                  juce::uint32 currentFrameIndex)
{
    std::unique_ptr<PendingProofDump> dump;

    {
        const juce::ScopedLock lock (proofDumpLock);
        dump = std::move (pendingProofDump);
        pendingProofDump.reset();
    }

    if (dump == nullptr)
        return;

    if (! dump->outputDirectory.createDirectory())
    {
        reportStatus ("proof dump failed: could not create " + dump->outputDirectory.getFullPathName());
        return;
    }

    const auto frameImage = capturedFrameToImage (renderBackend.captureFrame());
    const auto frameFile = dump->outputDirectory.getChildFile ("frame.png");
    const auto cookOrderFile = dump->outputDirectory.getChildFile ("cook_order.json");
    const auto nodeStatsFile = dump->outputDirectory.getChildFile ("node_stats.json");
    const auto loudnessCompoundFile = dump->outputDirectory.getChildFile ("loudness_compound.json");
    const auto runtimeRegistryFile = dump->outputDirectory.getChildFile ("runtime_registry.json");
    const auto runtimeOpCatalogFile = dump->outputDirectory.getChildFile ("runtime_op_catalog.json");
    const auto runtimeOpCoverageFile = dump->outputDirectory.getChildFile ("runtime_op_coverage.json");
    const auto runtimeUiDiagnosticsFile = dump->outputDirectory.getChildFile ("runtime_ui_diagnostics.json");
    const auto runtimeDryRunFile = dump->outputDirectory.getChildFile ("runtime_dry_run.json");
    const auto runtimeExecutionFile = dump->outputDirectory.getChildFile ("runtime_execution.json");
    const auto missingRuntimeOpRegistryFile = dump->outputDirectory.getChildFile ("runtime_missing_runtimeop_registry.json");
    const auto missingRuntimeOpCoverageFile = dump->outputDirectory.getChildFile ("runtime_missing_runtimeop_coverage.json");
    const auto missingRuntimeOpDryRunFile = dump->outputDirectory.getChildFile ("runtime_missing_runtimeop_dry_run.json");
    const auto missingRuntimeOpExecutionFile = dump->outputDirectory.getChildFile ("runtime_missing_runtimeop_execution.json");
    const auto runtimeRegistry = loadVisibleRuntimeRegistry();
    const auto runtimeOpCatalog = makeRuntimeOpCatalog();
    const auto runtimeOpCoverage = inspectRuntimeOpCoverage (runtimeRegistry);
    const auto runtimeDryRun = dryRunRuntimeRegistry (runtimeRegistry);
    RuntimeSyntheticAudioInput syntheticRuntimeInput;
    syntheticRuntimeInput.channels = {
        { 0.0f, 1.0f, -1.0f, 0.0f },
        { 0.0f, 0.5f, -0.5f, 0.0f }
    };
    syntheticRuntimeInput.analysisGain = 1.5f;
    const auto runtimeExecution = executeRuntimeRegistryWithSyntheticAudio (runtimeRegistry,
                                                                            syntheticRuntimeInput);
    const auto missingRuntimeOpRegistry = loadRuntimeRegistryFromCandidates (missingRuntimeOpModuleLibraryPath());
    const auto missingRuntimeOpCoverage = missingRuntimeOpRegistry.ok
                                              ? inspectRuntimeOpCoverage (missingRuntimeOpRegistry.registry)
                                              : RuntimeOpCoverageResult {};
    auto runtimeUiDiagnostics = runtimeOpDiagnostics.empty()
                                    ? makeRuntimeOpModuleDiagnostics (runtimeOpCoverage.snapshot)
                                    : runtimeOpDiagnostics;
    appendRuntimeOpModuleDiagnostics (runtimeUiDiagnostics, missingRuntimeOpCoverage);
    const auto missingRuntimeOpDryRun = missingRuntimeOpRegistry.ok
                                            ? dryRunRuntimeRegistry (missingRuntimeOpRegistry.registry)
                                            : RuntimeDryRunResult {};
    const auto missingRuntimeOpExecution = missingRuntimeOpRegistry.ok
                                               ? executeRuntimeRegistryWithSyntheticAudio (
                                                   missingRuntimeOpRegistry.registry,
                                                   syntheticRuntimeInput)
                                               : RuntimeExecutionResult {};

    const auto cookOrderWritten = writeTextFile (cookOrderFile, makeCookOrderJson (dump->graph));
    const auto nodeStatsWritten = writeTextFile (nodeStatsFile,
                                                 makeNodeStatsJson (dump->graph,
                                                                    width,
                                                                    height,
                                                                    currentFrameIndex,
                                                                    timeSeconds,
                                                                    renderBackend.backendName(),
                                                                    renderBackend.lastStatus()));
    const auto loudnessCompoundWritten = writeTextFile (loudnessCompoundFile,
                                                        makeCompoundPatchJson (loudnessCompound));
    const auto runtimeRegistryWritten = writeTextFile (runtimeRegistryFile,
                                                       makeRuntimeRegistryJson (runtimeRegistry));
    const auto runtimeOpCatalogWritten = writeTextFile (runtimeOpCatalogFile,
                                                        makeRuntimeOpCatalogJson (runtimeOpCatalog));
    const auto runtimeOpCoverageWritten = runtimeOpCoverage.ok
                                              && writeTextFile (
                                                  runtimeOpCoverageFile,
                                                  makeRuntimeOpCoverageJson (runtimeOpCoverage.snapshot));
    const auto runtimeUiDiagnosticsWritten = writeTextFile (
        runtimeUiDiagnosticsFile,
        makeRuntimeOpModuleDiagnosticsJson (runtimeUiDiagnostics));
    const auto runtimeDryRunWritten = runtimeDryRun.ok
                                          && writeTextFile (runtimeDryRunFile,
                                                            makeRuntimeDryRunJson (runtimeDryRun.snapshot));
    const auto runtimeExecutionWritten = runtimeExecution.ok
                                             && writeTextFile (runtimeExecutionFile,
                                                               makeRuntimeExecutionJson (runtimeExecution.snapshot));
    const auto missingRuntimeOpRegistryWritten = missingRuntimeOpRegistry.ok
                                                     && writeTextFile (
                                                         missingRuntimeOpRegistryFile,
                                                         makeRuntimeRegistryJson (missingRuntimeOpRegistry.registry));
    const auto missingRuntimeOpCoverageWritten = missingRuntimeOpRegistry.ok
                                                     && ! missingRuntimeOpCoverage.ok
                                                     && writeTextFile (
                                                         missingRuntimeOpCoverageFile,
                                                         makeRuntimeOpCoverageJson (missingRuntimeOpCoverage.snapshot));
    const auto missingRuntimeOpDryRunWritten = missingRuntimeOpRegistry.ok
                                                   && ! missingRuntimeOpDryRun.ok
                                                   && writeTextFile (
                                                       missingRuntimeOpDryRunFile,
                                                       makeRuntimeDryRunJson (missingRuntimeOpDryRun.snapshot));
    const auto missingRuntimeOpExecutionWritten = missingRuntimeOpRegistry.ok
                                                      && ! missingRuntimeOpExecution.ok
                                                      && writeTextFile (
                                                          missingRuntimeOpExecutionFile,
                                                          makeRuntimeExecutionJson (missingRuntimeOpExecution.snapshot));
    const auto frameWritten = writePngFile (frameFile, frameImage);

    if (cookOrderWritten
        && nodeStatsWritten
        && loudnessCompoundWritten
        && runtimeRegistryWritten
        && runtimeOpCatalogWritten
        && runtimeOpCoverageWritten
        && runtimeUiDiagnosticsWritten
        && runtimeDryRunWritten
        && runtimeExecutionWritten
        && missingRuntimeOpRegistryWritten
        && missingRuntimeOpCoverageWritten
        && missingRuntimeOpDryRunWritten
        && missingRuntimeOpExecutionWritten
        && frameWritten)
    {
        reportStatus ("proof dumped: " + dump->outputDirectory.getFullPathName());
        return;
    }

    reportStatus ("proof dump failed: "
                  + juce::String (cookOrderWritten ? "" : "cook_order.json ")
                  + juce::String (nodeStatsWritten ? "" : "node_stats.json ")
                  + juce::String (loudnessCompoundWritten ? "" : "loudness_compound.json ")
                  + juce::String (runtimeRegistryWritten ? "" : "runtime_registry.json ")
                  + juce::String (runtimeOpCatalogWritten ? "" : "runtime_op_catalog.json ")
                  + juce::String (runtimeOpCoverageWritten ? "" : "runtime_op_coverage.json ")
                  + juce::String (runtimeUiDiagnosticsWritten ? "" : "runtime_ui_diagnostics.json ")
                  + juce::String (runtimeDryRunWritten ? "" : "runtime_dry_run.json ")
                  + juce::String (runtimeExecutionWritten ? "" : "runtime_execution.json ")
                  + juce::String (missingRuntimeOpRegistryWritten ? "" : "runtime_missing_runtimeop_registry.json ")
                  + juce::String (missingRuntimeOpCoverageWritten ? "" : "runtime_missing_runtimeop_coverage.json ")
                  + juce::String (missingRuntimeOpDryRunWritten ? "" : "runtime_missing_runtimeop_dry_run.json ")
                  + juce::String (missingRuntimeOpExecutionWritten ? "" : "runtime_missing_runtimeop_execution.json ")
                  + juce::String (frameWritten ? "" : "frame.png"));
}

juce::Image OpenGLShaderPreview::capturedFrameToImage (const CapturedFrame& frame) const
{
    juce::Image image (juce::Image::ARGB, frame.width, frame.height, true);
    juce::Image::BitmapData bitmap (image, juce::Image::BitmapData::writeOnly);

    for (int y = 0; y < frame.height; ++y)
    {
        const auto sourceY = frame.height - 1 - y;

        for (int x = 0; x < frame.width; ++x)
        {
            const auto sourceIndex = (static_cast<size_t> (sourceY) * static_cast<size_t> (frame.width)
                                      + static_cast<size_t> (x)) * 4u;

            bitmap.setPixelColour (x,
                                   y,
                                   juce::Colour::fromRGBA (frame.rgba[sourceIndex],
                                                           frame.rgba[sourceIndex + 1],
                                                           frame.rgba[sourceIndex + 2],
                                                           frame.rgba[sourceIndex + 3]));
        }
    }

    return image;
}

void OpenGLShaderPreview::releaseGLObjects()
{
    imguiOverlay.shutdown();
    renderBackend.release();
}

void OpenGLShaderPreview::reportStatus (juce::String message)
{
    lastStatus = message;

    if (onStatusMessage != nullptr)
        onStatusMessage (std::move (message));
}

void OpenGLShaderPreview::updateImGuiMousePosition (const juce::MouseEvent& event)
{
    imguiOverlay.setMousePosition (event.position.x, event.position.y);
}

}
