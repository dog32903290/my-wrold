#include "V1ShaderProofArtifacts.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <system_error>

namespace myworld
{
namespace
{
constexpr const char* defaultModuleLibraryPath = "fixtures/module-libraries/default.module-library.json";
constexpr const char* missingRuntimeOpModuleLibraryPath = "fixtures/module-libraries/missing-runtimeop.module-library.json";

std::vector<std::filesystem::path> candidatePaths (const std::vector<std::filesystem::path>& roots,
                                                   const char* relativePath)
{
    std::vector<std::filesystem::path> paths;

    for (const auto& root : roots)
    {
        if (! root.empty())
            paths.push_back (root / relativePath);
    }

    paths.push_back (std::filesystem::current_path() / relativePath);
    paths.push_back (std::filesystem::path (relativePath));

    std::vector<std::filesystem::path> uniquePaths;
    for (const auto& path : paths)
    {
        if (std::find (uniquePaths.begin(), uniquePaths.end(), path) == uniquePaths.end())
            uniquePaths.push_back (path);
    }

    return uniquePaths;
}

RuntimeRegistryLoadResult loadRuntimeRegistryFromCandidates (const std::vector<std::filesystem::path>& candidateRoots,
                                                             const char* relativePath)
{
    std::string lastError;

    for (const auto& path : candidatePaths (candidateRoots, relativePath))
    {
        const auto registry = loadRuntimeRegistryFromModuleLibrary (path.string());
        if (registry.ok)
            return registry;

        lastError = registry.error;
    }

    return { false, {}, lastError.empty() ? "could not load module library: " + std::string (relativePath)
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

bool writeTextFile (const std::filesystem::path& path, const std::string& text)
{
    std::error_code error;
    std::filesystem::create_directories (path.parent_path(), error);
    if (error)
        return false;

    juce::File file (path.string());
    return file.replaceWithText (juce::String::fromUTF8 (text.data(), static_cast<int> (text.size())),
                                 false,
                                 false,
                                 "\n");
}

bool writePngFile (const std::filesystem::path& path, const juce::Image& image)
{
    std::error_code error;
    std::filesystem::create_directories (path.parent_path(), error);
    if (error)
        return false;

    juce::File file (path.string());
    if (file.existsAsFile() && ! file.deleteFile())
        return false;

    auto output = file.createOutputStream();
    if (output == nullptr)
        return false;

    juce::PNGImageFormat pngFormat;
    return pngFormat.writeImageToStream (image, *output);
}

bool hasVisualReactionFrames (const V1ShaderProofArtifactRequest& request)
{
    return request.quietFrameImage.isValid()
        && request.loudFrameImage.isValid()
        && request.quietFrameImage.getWidth() == request.loudFrameImage.getWidth()
        && request.quietFrameImage.getHeight() == request.loudFrameImage.getHeight();
}

std::string makeVisualReactionJson (const V1ShaderProofArtifactRequest& request)
{
    const auto width = request.quietFrameImage.getWidth();
    const auto height = request.quietFrameImage.getHeight();
    const auto pixelCount = width * height;
    int changedPixels = 0;
    double deltaSum = 0.0;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const auto quiet = request.quietFrameImage.getPixelAt (x, y);
            const auto loud = request.loudFrameImage.getPixelAt (x, y);
            const auto delta = (std::abs (static_cast<int> (quiet.getRed()) - static_cast<int> (loud.getRed()))
                              + std::abs (static_cast<int> (quiet.getGreen()) - static_cast<int> (loud.getGreen()))
                              + std::abs (static_cast<int> (quiet.getBlue()) - static_cast<int> (loud.getBlue())))
                              / (3.0 * 255.0);
            if (delta > 0.0)
                ++changedPixels;
            deltaSum += delta;
        }
    }

    const auto loudnessDelta = request.loudLoudness - request.quietLoudness;
    const auto meanAbsDelta = pixelCount > 0 ? deltaSum / static_cast<double> (pixelCount) : 0.0;
    const auto ok = pixelCount > 0 && loudnessDelta != 0.0f && changedPixels > 0;

    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"v1VisualReactionProof\",\n";
    out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
    out << "  \"status\": " << (ok ? "\"changed\"" : "\"unchanged\"") << ",\n";
    out << "  \"quietLoudness\": " << request.quietLoudness << ",\n";
    out << "  \"loudLoudness\": " << request.loudLoudness << ",\n";
    out << "  \"loudnessDelta\": " << loudnessDelta << ",\n";
    out << "  \"width\": " << width << ",\n";
    out << "  \"height\": " << height << ",\n";
    out << "  \"pixelCount\": " << pixelCount << ",\n";
    out << "  \"changedPixels\": " << changedPixels << ",\n";
    out << "  \"meanAbsDelta\": " << meanAbsDelta << "\n";
    out << "}\n";
    return out.str();
}

V1ShaderProofArtifactResult makeInitialResult (const V1ShaderProofArtifactRequest& request)
{
    V1ShaderProofArtifactResult result;
    result.outputDirectory = request.outputDirectory;
    result.artifactPaths = {
        request.outputDirectory / "cook_order.json",
        request.outputDirectory / "frame.png",
        request.outputDirectory / "loudness_compound.json",
        request.outputDirectory / "node_stats.json",
        request.outputDirectory / "runtime_dry_run.json",
        request.outputDirectory / "runtime_execution.json",
        request.outputDirectory / "runtime_missing_runtimeop_coverage.json",
        request.outputDirectory / "runtime_missing_runtimeop_dry_run.json",
        request.outputDirectory / "runtime_missing_runtimeop_execution.json",
        request.outputDirectory / "runtime_missing_runtimeop_registry.json",
        request.outputDirectory / "runtime_op_catalog.json",
        request.outputDirectory / "runtime_op_coverage.json",
        request.outputDirectory / "runtime_registry.json",
        request.outputDirectory / "runtime_ui_diagnostics.json"
    };
    if (hasVisualReactionFrames (request))
        result.artifactPaths.push_back (request.outputDirectory / "visual_reaction.json");
    return result;
}

std::string makeFailureMessage (const std::vector<std::string>& missing)
{
    std::ostringstream out;
    out << "proof dump failed: ";

    for (const auto& item : missing)
        out << item << ' ';

    return out.str();
}
}

V1ShaderProofArtifactResult writeV1ShaderProofArtifacts (const V1ShaderProofArtifactRequest& request)
{
    auto result = makeInitialResult (request);

    std::error_code createError;
    std::filesystem::create_directories (request.outputDirectory, createError);
    if (createError)
    {
        result.status = "failed";
        result.error = "could not create " + request.outputDirectory.string() + ": " + createError.message();
        return result;
    }

    const auto runtimeRegistry = loadRuntimeRegistryFromCandidates (request.candidateRoots, defaultModuleLibraryPath);
    if (! runtimeRegistry.ok)
    {
        result.status = "failed";
        result.error = runtimeRegistry.error;
        return result;
    }

    const auto runtimeOpCatalog = makeRuntimeOpCatalog();
    const auto runtimeOpCoverage = inspectRuntimeOpCoverage (runtimeRegistry.registry);
    const auto runtimeDryRun = dryRunRuntimeRegistry (runtimeRegistry.registry);
    RuntimeSyntheticAudioInput syntheticRuntimeInput;
    syntheticRuntimeInput.channels = {
        { 0.0f, 1.0f, -1.0f, 0.0f },
        { 0.0f, 0.5f, -0.5f, 0.0f }
    };
    syntheticRuntimeInput.analysisGain = 1.5f;
    const auto runtimeExecution = executeRuntimeRegistryWithSyntheticAudio (runtimeRegistry.registry,
                                                                            syntheticRuntimeInput);
    const auto missingRuntimeOpRegistry = loadRuntimeRegistryFromCandidates (request.candidateRoots,
                                                                             missingRuntimeOpModuleLibraryPath);
    const auto missingRuntimeOpCoverage = missingRuntimeOpRegistry.ok
                                              ? inspectRuntimeOpCoverage (missingRuntimeOpRegistry.registry)
                                              : RuntimeOpCoverageResult {};
    auto runtimeUiDiagnostics = request.runtimeOpDiagnostics.empty()
                                    ? makeRuntimeOpModuleDiagnostics (runtimeOpCoverage.snapshot)
                                    : request.runtimeOpDiagnostics;
    appendRuntimeOpModuleDiagnostics (runtimeUiDiagnostics, missingRuntimeOpCoverage);
    const auto missingRuntimeOpDryRun = missingRuntimeOpRegistry.ok
                                            ? dryRunRuntimeRegistry (missingRuntimeOpRegistry.registry)
                                            : RuntimeDryRunResult {};
    const auto missingRuntimeOpExecution = missingRuntimeOpRegistry.ok
                                               ? executeRuntimeRegistryWithSyntheticAudio (
                                                   missingRuntimeOpRegistry.registry,
                                                   syntheticRuntimeInput)
                                               : RuntimeExecutionResult {};

    const auto cookOrderWritten = writeTextFile (request.outputDirectory / "cook_order.json",
                                                 makeCookOrderJson (request.graph));
    const auto nodeStatsWritten = writeTextFile (
        request.outputDirectory / "node_stats.json",
        makeNodeStatsJson (request.graph,
                           request.viewportWidth,
                           request.viewportHeight,
                           request.frameIndex,
                           request.timeSeconds,
                           request.backendName,
                           request.backendStatus));
    const auto loudnessCompoundWritten = writeTextFile (request.outputDirectory / "loudness_compound.json",
                                                        makeCompoundPatchJson (request.loudnessCompound));
    const auto runtimeRegistryWritten = writeTextFile (request.outputDirectory / "runtime_registry.json",
                                                       makeRuntimeRegistryJson (runtimeRegistry.registry));
    const auto runtimeOpCatalogWritten = writeTextFile (request.outputDirectory / "runtime_op_catalog.json",
                                                        makeRuntimeOpCatalogJson (runtimeOpCatalog));
    const auto runtimeOpCoverageWritten = runtimeOpCoverage.ok
                                          && writeTextFile (
                                              request.outputDirectory / "runtime_op_coverage.json",
                                              makeRuntimeOpCoverageJson (runtimeOpCoverage.snapshot));
    const auto runtimeUiDiagnosticsWritten = writeTextFile (
        request.outputDirectory / "runtime_ui_diagnostics.json",
        makeRuntimeOpModuleDiagnosticsJson (runtimeUiDiagnostics));
    const auto runtimeDryRunWritten = runtimeDryRun.ok
                                      && writeTextFile (request.outputDirectory / "runtime_dry_run.json",
                                                        makeRuntimeDryRunJson (runtimeDryRun.snapshot));
    const auto runtimeExecutionWritten = runtimeExecution.ok
                                         && writeTextFile (
                                             request.outputDirectory / "runtime_execution.json",
                                             makeRuntimeExecutionJson (runtimeExecution.snapshot));
    const auto missingRuntimeOpRegistryWritten = missingRuntimeOpRegistry.ok
                                                 && writeTextFile (
                                                     request.outputDirectory / "runtime_missing_runtimeop_registry.json",
                                                     makeRuntimeRegistryJson (missingRuntimeOpRegistry.registry));
    const auto missingRuntimeOpCoverageWritten = missingRuntimeOpRegistry.ok
                                                 && ! missingRuntimeOpCoverage.ok
                                                 && writeTextFile (
                                                     request.outputDirectory / "runtime_missing_runtimeop_coverage.json",
                                                     makeRuntimeOpCoverageJson (missingRuntimeOpCoverage.snapshot));
    const auto missingRuntimeOpDryRunWritten = missingRuntimeOpRegistry.ok
                                               && ! missingRuntimeOpDryRun.ok
                                               && writeTextFile (
                                                   request.outputDirectory / "runtime_missing_runtimeop_dry_run.json",
                                                   makeRuntimeDryRunJson (missingRuntimeOpDryRun.snapshot));
    const auto missingRuntimeOpExecutionWritten = missingRuntimeOpRegistry.ok
                                                  && ! missingRuntimeOpExecution.ok
                                                  && writeTextFile (
                                                      request.outputDirectory / "runtime_missing_runtimeop_execution.json",
                                                      makeRuntimeExecutionJson (missingRuntimeOpExecution.snapshot));
    const auto visualReactionWritten = ! hasVisualReactionFrames (request)
                                       || writeTextFile (request.outputDirectory / "visual_reaction.json",
                                                         makeVisualReactionJson (request));
    const auto frameWritten = writePngFile (request.outputDirectory / "frame.png", request.frameImage);

    std::vector<std::string> missing;
    if (! cookOrderWritten) missing.push_back ("cook_order.json");
    if (! nodeStatsWritten) missing.push_back ("node_stats.json");
    if (! loudnessCompoundWritten) missing.push_back ("loudness_compound.json");
    if (! runtimeRegistryWritten) missing.push_back ("runtime_registry.json");
    if (! runtimeOpCatalogWritten) missing.push_back ("runtime_op_catalog.json");
    if (! runtimeOpCoverageWritten) missing.push_back ("runtime_op_coverage.json");
    if (! runtimeUiDiagnosticsWritten) missing.push_back ("runtime_ui_diagnostics.json");
    if (! runtimeDryRunWritten) missing.push_back ("runtime_dry_run.json");
    if (! runtimeExecutionWritten) missing.push_back ("runtime_execution.json");
    if (! missingRuntimeOpRegistryWritten) missing.push_back ("runtime_missing_runtimeop_registry.json");
    if (! missingRuntimeOpCoverageWritten) missing.push_back ("runtime_missing_runtimeop_coverage.json");
    if (! missingRuntimeOpDryRunWritten) missing.push_back ("runtime_missing_runtimeop_dry_run.json");
    if (! missingRuntimeOpExecutionWritten) missing.push_back ("runtime_missing_runtimeop_execution.json");
    if (! visualReactionWritten) missing.push_back ("visual_reaction.json");
    if (! frameWritten) missing.push_back ("frame.png");

    result.ok = missing.empty();
    result.status = result.ok ? "dumped" : "failed";
    result.error = result.ok ? std::string {} : makeFailureMessage (missing);
    return result;
}
}
