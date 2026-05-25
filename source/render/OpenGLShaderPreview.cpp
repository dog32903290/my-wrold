#include "OpenGLShaderPreview.h"

#include "CompoundModule.h"
#include "RuntimeRegistry.h"
#include "V1ShaderProofArtifacts.h"

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <iterator>
#include <thread>
#include <vector>

namespace myworld
{
namespace
{
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

std::vector<std::filesystem::path> proofCandidateRoots()
{
    const auto executableDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();

    return {
        juce::File::getCurrentWorkingDirectory().getFullPathName().toStdString(),
        parentDirectory (executableDir, 5).getFullPathName().toStdString()
    };
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

void OpenGLShaderPreview::setInputSnapshot (const ShaderPreviewInputSnapshot& snapshot)
{
    const auto input = makeRenderFrameInputFromShaderPreviewInput (
        snapshot,
        0.0,
        frameIndex,
        loudness.load (std::memory_order_relaxed));
    setLoudness (input.loudness);
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
    handlePendingProofDump (juce::jmax (1, width), juce::jmax (1, height), elapsed, frameIndex);

    const auto deltaSeconds = static_cast<float> (nowSeconds - lastFrameSeconds);
    lastFrameSeconds = nowSeconds;
    imguiOverlay.beginFrame (juce::jmax (1, width), juce::jmax (1, height), scale, deltaSeconds);
    imguiOverlay.drawSmokePanel (seedNodeSpecs, loudnessCompound, lastStatus.toStdString());
    imguiOverlay.render();

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

    const auto activeLoudness = loudness.load (std::memory_order_relaxed);
    const auto frameImage = capturedFrameToImage (renderBackend.captureFrame());
    renderBackend.renderFrame ({ timeSeconds, currentFrameIndex, 0.0f });
    const auto quietFrameImage = capturedFrameToImage (renderBackend.captureFrame());
    renderBackend.renderFrame ({ timeSeconds, currentFrameIndex, 1.0f });
    const auto loudFrameImage = capturedFrameToImage (renderBackend.captureFrame());
    renderBackend.renderFrame ({ timeSeconds, currentFrameIndex, activeLoudness });

    V1ShaderProofArtifactRequest request;
    request.outputDirectory = dump->outputDirectory.getFullPathName().toStdString();
    request.candidateRoots = proofCandidateRoots();
    request.graph = dump->graph;
    request.frameImage = frameImage;
    request.viewportWidth = width;
    request.viewportHeight = height;
    request.timeSeconds = timeSeconds;
    request.frameIndex = currentFrameIndex;
    request.backendName = renderBackend.backendName();
    request.backendStatus = renderBackend.lastStatus();
    request.quietFrameImage = quietFrameImage;
    request.loudFrameImage = loudFrameImage;
    request.quietLoudness = 0.0f;
    request.loudLoudness = 1.0f;
    request.loudnessCompound = loudnessCompound;
    request.runtimeOpDiagnostics = runtimeOpDiagnostics;

    const auto outputDirectoryName = dump->outputDirectory.getFullPathName();
    std::thread ([safe = juce::Component::SafePointer<OpenGLShaderPreview> (this),
                  outputDirectoryName,
                  proofRequest = std::move (request)]() mutable
    {
        const auto result = writeV1ShaderProofArtifacts (proofRequest);
        juce::MessageManager::callAsync ([safe, outputDirectoryName, result]
        {
            if (safe == nullptr)
                return;

            if (result.ok)
            {
                safe->reportStatus ("proof dumped: " + outputDirectoryName);
                return;
            }

            safe->reportStatus (juce::String (result.error));
        });
    }).detach();
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
