#include "RuntimeRegistry.h"

#include "AudioAnalyzerState.h"
#include "CompoundPatch.h"
#include "StorageContract.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace myworld
{
namespace
{
std::string jsonEscaped (const std::string& text)
{
    std::ostringstream out;

    for (const auto character : text)
    {
        switch (character)
        {
            case '"':  out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (static_cast<unsigned char> (character) < 0x20)
                    out << "\\u" << std::hex << std::setw (4) << std::setfill ('0')
                        << static_cast<int> (static_cast<unsigned char> (character));
                else
                    out << character;
                break;
        }
    }

    return out.str();
}

void appendStringArray (std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";

    for (size_t index = 0; index < values.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << "\"" << jsonEscaped (values[index]) << "\"";
    }

    out << "]";
}

void appendValueObject (std::ostringstream& out, const std::vector<RuntimeOutputValue>& values)
{
    out << "{";

    for (size_t index = 0; index < values.size(); ++index)
    {
        if (index != 0)
            out << ",";

        out << " \"" << jsonEscaped (values[index].id) << "\": " << values[index].value;
    }

    if (! values.empty())
        out << " ";

    out << "}";
}

std::string fallback (const std::string& value, const std::string& fallbackValue)
{
    return value.empty() ? fallbackValue : value;
}

std::string resolvePathNear (const std::string& anchorPath, const std::string& candidatePath)
{
    namespace fs = std::filesystem;

    const fs::path candidate { candidatePath };
    if (candidate.is_absolute() || fs::exists (candidate))
        return candidate.string();

    auto directory = fs::path { anchorPath };
    directory = directory.has_parent_path() ? directory.parent_path() : fs::current_path();

    for (int depth = 0; depth < 8; ++depth)
    {
        const auto resolved = directory / candidate;
        if (fs::exists (resolved))
            return resolved.string();

        if (! directory.has_parent_path() || directory == directory.parent_path())
            break;

        directory = directory.parent_path();
    }

    return candidate.string();
}

std::vector<std::string> portIds (const std::vector<CompoundPublicPort>& ports)
{
    std::vector<std::string> ids;
    ids.reserve (ports.size());

    for (const auto& port : ports)
        ids.push_back (port.id);

    return ids;
}

std::vector<RuntimeRegistryChild> runtimeChildren (const CompoundPatchSpec& compound)
{
    std::vector<RuntimeRegistryChild> children;
    children.reserve (compound.children.size());

    for (const auto& child : compound.children)
        children.push_back ({ child.id, child.nodeType, child.role });

    return children;
}

const RuntimeRegistryChild* findRuntimeChild (const RuntimeRegistryEntry& entry, const std::string& childId)
{
    const auto found = std::find_if (entry.children.begin(), entry.children.end(), [&childId] (const auto& child) {
        return child.id == childId;
    });

    return found == entry.children.end() ? nullptr : &(*found);
}

struct MonoMixResult
{
    std::vector<float> samples;
    double rms = 0.0;
    double peak = 0.0;
};

MonoMixResult mixToMono (const std::vector<std::vector<float>>& channels)
{
    MonoMixResult result;

    if (channels.empty() || channels.front().empty())
        return result;

    const auto sampleCount = channels.front().size();
    result.samples.resize (sampleCount, 0.0f);

    for (size_t sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
    {
        double mono = 0.0;

        for (const auto& channel : channels)
            mono += static_cast<double> (channel[sampleIndex]);

        result.samples[sampleIndex] = static_cast<float> (mono / static_cast<double> (channels.size()));
    }

    double sumSquares = 0.0;
    double peak = 0.0;

    for (const auto sample : result.samples)
    {
        const auto value = static_cast<double> (sample);
        sumSquares += value * value;
        peak = std::max (peak, std::abs (value));
    }

    result.rms = std::sqrt (sumSquares / static_cast<double> (sampleCount));
    result.peak = peak;
    return result;
}

RuntimeRegistryEntry makeRuntimeRegistryEntry (const ModulePackageManifest& module, const CompoundPatchSpec& compound)
{
    return {
        fallback (module.nodeType, compound.type),
        fallback (module.title, compound.displayName),
        fallback (module.runtimeDomain, "graph"),
        "compound.patch",
        fallback (module.previewPolicy, "none"),
        runtimeChildren (compound),
        compound.children.size(),
        compound.internalEdges.size(),
        portIds (compound.publicInputs),
        portIds (compound.publicOutputs),
        makeCompoundCookOrder (compound)
    };
}
}

RuntimeRegistryLoadResult loadRuntimeRegistryFromModuleLibrary (const std::string& libraryPath)
{
    const auto resolvedLibraryPath = resolvePathNear ({}, libraryPath);
    const auto library = loadModuleLibraryManifest (resolvedLibraryPath);
    if (! library.ok)
        return { false, {}, library.error };

    RuntimeRegistry registry;

    for (const auto& modulePath : library.manifest.modulePackages)
    {
        const auto resolvedModulePath = resolvePathNear (resolvedLibraryPath, modulePath);
        const auto module = loadModulePackageManifest (resolvedModulePath);
        if (! module.ok)
            return { false, {}, module.error };

        const auto resolvedCompoundPath = resolvePathNear (resolvedModulePath, module.manifest.patchPath);
        const auto compound = loadCompoundPatchSpec (resolvedCompoundPath);
        if (! compound.ok)
            return { false, {}, compound.error };

        if (! isValidCompoundPatchSpec (compound.spec))
            return { false, {}, "compound patch is not valid for runtime registry: " + compound.spec.type };

        registry.entries.push_back (makeRuntimeRegistryEntry (module.manifest, compound.spec));
    }

    return { true, registry, {} };
}

RuntimeDryRunResult dryRunRuntimeRegistry (const RuntimeRegistry& registry)
{
    RuntimeDryRunSnapshot snapshot;
    snapshot.version = registry.version;

    for (const auto& entry : registry.entries)
    {
        RuntimeEntryDryRunStatus entryStatus;
        entryStatus.nodeType = entry.nodeType;
        entryStatus.executionKind = entry.executionKind;
        entryStatus.status = "dry-run-ready";
        entryStatus.children.reserve (entry.cookOrder.size());

        for (size_t cookIndex = 0; cookIndex < entry.cookOrder.size(); ++cookIndex)
        {
            const auto& childId = entry.cookOrder[cookIndex];
            const auto* child = findRuntimeChild (entry, childId);

            if (child == nullptr)
                return { false, {}, "dry-run missing child metadata for " + entry.nodeType + ":" + childId };

            entryStatus.children.push_back ({ cookIndex,
                                             child->id,
                                             child->nodeType,
                                             child->role,
                                             "dry-run-ready",
                                             "validated child order; RuntimeOp not executed" });
        }

        if (entryStatus.children.size() != entry.childCount)
            return { false, {}, "dry-run child count mismatch for " + entry.nodeType };

        snapshot.entries.push_back (entryStatus);
    }

    return { true, snapshot, {} };
}

RuntimeExecutionResult executeRuntimeRegistryWithSyntheticAudio (const RuntimeRegistry& registry,
                                                                 const std::vector<float>& samples,
                                                                 const float analysisGain)
{
    RuntimeSyntheticAudioInput input;
    input.channels.push_back (samples);
    input.analysisGain = analysisGain;
    return executeRuntimeRegistryWithSyntheticAudio (registry, input);
}

RuntimeExecutionResult executeRuntimeRegistryWithSyntheticAudio (const RuntimeRegistry& registry,
                                                                 const RuntimeSyntheticAudioInput& input)
{
    if (input.channels.empty())
        return { false, {}, "synthetic audio execution requires at least one channel" };

    const auto sampleCount = input.channels.front().size();

    if (sampleCount == 0)
        return { false, {}, "synthetic audio execution requires at least one sample" };

    if (! std::isfinite (input.analysisGain))
        return { false, {}, "synthetic audio execution requires a finite analysis gain" };

    for (const auto& channel : input.channels)
    {
        if (channel.size() != sampleCount)
            return { false, {}, "synthetic audio execution requires equal channel lengths" };
    }

    RuntimeExecutionSnapshot snapshot;
    snapshot.version = registry.version;

    for (const auto& entry : registry.entries)
    {
        RuntimeEntryExecutionStatus entryStatus;
        entryStatus.nodeType = entry.nodeType;
        entryStatus.executionKind = entry.executionKind;
        entryStatus.status = "partial-execution";
        entryStatus.children.reserve (entry.cookOrder.size());

        MonoMixResult monoMix;
        bool hasAudioInput = false;
        bool hasMonoMix = false;
        bool hasRms = false;
        double rmsOutput = 0.0;

        for (size_t cookIndex = 0; cookIndex < entry.cookOrder.size(); ++cookIndex)
        {
            const auto& childId = entry.cookOrder[cookIndex];
            const auto* child = findRuntimeChild (entry, childId);

            if (child == nullptr)
                return { false, {}, "execution missing child metadata for " + entry.nodeType + ":" + childId };

            RuntimeChildExecutionStatus childStatus {
                cookIndex,
                child->id,
                child->nodeType,
                child->role,
                "not-executed",
                "RuntimeOp not implemented for " + child->nodeType,
                {},
                {}
            };

            if (child->nodeType == "audio.input")
            {
                hasAudioInput = true;
                childStatus.status = "computed";
                childStatus.reason = "accepted synthetic audio channels";
                childStatus.outputs = {
                    { "channelCount", static_cast<double> (input.channels.size()) },
                    { "sampleCount", static_cast<double> (sampleCount) }
                };
            }
            else if (child->nodeType == "audio.mono_mix")
            {
                childStatus.inputs = {
                    { "channelCount", static_cast<double> (input.channels.size()) },
                    { "sampleCount", static_cast<double> (sampleCount) }
                };

                if (hasAudioInput)
                {
                    monoMix = mixToMono (input.channels);
                    hasMonoMix = true;
                    childStatus.status = "computed";
                    childStatus.reason = "averaged synthetic channels into mono samples";
                    childStatus.outputs = {
                        { "sampleCount", static_cast<double> (monoMix.samples.size()) },
                        { "rms", monoMix.rms },
                        { "peak", monoMix.peak }
                    };
                }
                else
                {
                    childStatus.status = "blocked";
                    childStatus.reason = "waiting for audio.input output";
                }
            }
            else if (child->nodeType == "analyzer.rms")
            {
                childStatus.inputs = {
                    { "sampleCount", static_cast<double> (monoMix.samples.size()) },
                    { "sourceRms", monoMix.rms },
                    { "sourcePeak", monoMix.peak }
                };

                if (hasMonoMix)
                {
                    const float* channels[] { monoMix.samples.data() };
                    AudioAnalyzerState analyzer;
                    analyzer.processBlock (channels, 1, static_cast<int> (monoMix.samples.size()), 1.0f);
                    const auto analyzerSnapshot = analyzer.getSnapshot();

                    hasRms = true;
                    rmsOutput = static_cast<double> (analyzerSnapshot.rms);
                    childStatus.status = "computed";
                    childStatus.reason = "computed rms/peak from audio.mono_mix output";
                    childStatus.outputs = {
                        { "rms", rmsOutput },
                        { "peak", static_cast<double> (analyzerSnapshot.peak) }
                    };
                }
                else
                {
                    childStatus.status = "blocked";
                    childStatus.reason = "waiting for audio.mono_mix output";
                }
            }
            else if (child->nodeType == "analyzer.analysis_gain")
            {
                childStatus.inputs = {
                    { "input", rmsOutput },
                    { "gain", static_cast<double> (input.analysisGain) }
                };

                if (hasRms)
                {
                    childStatus.status = "computed";
                    childStatus.reason = "calibrated analyzer.rms output with analysis gain";
                    childStatus.outputs = {
                        { "out", rmsOutput * static_cast<double> (input.analysisGain) }
                    };
                }
                else
                {
                    childStatus.status = "blocked";
                    childStatus.reason = "waiting for analyzer.rms output";
                }
            }

            entryStatus.children.push_back (childStatus);
        }

        snapshot.entries.push_back (entryStatus);
    }

    return { true, snapshot, {} };
}

std::string makeRuntimeRegistryJson (const RuntimeRegistry& registry)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"runtimeRegistry\",\n";
    out << "  \"version\": " << registry.version << ",\n";
    out << "  \"entries\": [\n";

    for (size_t index = 0; index < registry.entries.size(); ++index)
    {
        const auto& entry = registry.entries[index];
        out << "    {\n";
        out << "      \"nodeType\": \"" << jsonEscaped (entry.nodeType) << "\",\n";
        out << "      \"displayName\": \"" << jsonEscaped (entry.displayName) << "\",\n";
        out << "      \"runtimeDomain\": \"" << jsonEscaped (entry.runtimeDomain) << "\",\n";
        out << "      \"executionKind\": \"" << jsonEscaped (entry.executionKind) << "\",\n";
        out << "      \"previewPolicy\": \"" << jsonEscaped (entry.previewPolicy) << "\",\n";
        out << "      \"childCount\": " << entry.childCount << ",\n";
        out << "      \"internalEdgeCount\": " << entry.internalEdgeCount << ",\n";
        out << "      \"children\": [\n";

        for (size_t childIndex = 0; childIndex < entry.children.size(); ++childIndex)
        {
            const auto& child = entry.children[childIndex];
            out << "        { \"id\": \"" << jsonEscaped (child.id)
                << "\", \"nodeType\": \"" << jsonEscaped (child.nodeType)
                << "\", \"role\": \"" << jsonEscaped (child.role) << "\" }";

            if (childIndex + 1 < entry.children.size())
                out << ",";

            out << "\n";
        }

        out << "      ],\n";
        out << "      \"publicInputs\": ";
        appendStringArray (out, entry.publicInputs);
        out << ",\n";
        out << "      \"publicOutputs\": ";
        appendStringArray (out, entry.publicOutputs);
        out << ",\n";
        out << "      \"cookOrder\": ";
        appendStringArray (out, entry.cookOrder);
        out << "\n";
        out << "    }";

        if (index + 1 < registry.entries.size())
            out << ",";

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
    return out.str();
}

std::string makeRuntimeDryRunJson (const RuntimeDryRunSnapshot& snapshot)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"runtimeDryRun\",\n";
    out << "  \"version\": " << snapshot.version << ",\n";
    out << "  \"mode\": \"" << jsonEscaped (snapshot.mode) << "\",\n";
    out << "  \"entries\": [\n";

    for (size_t entryIndex = 0; entryIndex < snapshot.entries.size(); ++entryIndex)
    {
        const auto& entry = snapshot.entries[entryIndex];
        out << "    {\n";
        out << "      \"nodeType\": \"" << jsonEscaped (entry.nodeType) << "\",\n";
        out << "      \"executionKind\": \"" << jsonEscaped (entry.executionKind) << "\",\n";
        out << "      \"status\": \"" << jsonEscaped (entry.status) << "\",\n";
        out << "      \"children\": [\n";

        for (size_t childIndex = 0; childIndex < entry.children.size(); ++childIndex)
        {
            const auto& child = entry.children[childIndex];
            out << "        {\n";
            out << "          \"cookIndex\": " << child.cookIndex << ",\n";
            out << "          \"childId\": \"" << jsonEscaped (child.childId) << "\",\n";
            out << "          \"nodeType\": \"" << jsonEscaped (child.nodeType) << "\",\n";
            out << "          \"role\": \"" << jsonEscaped (child.role) << "\",\n";
            out << "          \"status\": \"" << jsonEscaped (child.status) << "\",\n";
            out << "          \"reason\": \"" << jsonEscaped (child.reason) << "\"\n";
            out << "        }";

            if (childIndex + 1 < entry.children.size())
                out << ",";

            out << "\n";
        }

        out << "      ]\n";
        out << "    }";

        if (entryIndex + 1 < snapshot.entries.size())
            out << ",";

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
    return out.str();
}

std::string makeRuntimeExecutionJson (const RuntimeExecutionSnapshot& snapshot)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"runtimeExecution\",\n";
    out << "  \"version\": " << snapshot.version << ",\n";
    out << "  \"mode\": \"" << jsonEscaped (snapshot.mode) << "\",\n";
    out << "  \"entries\": [\n";

    for (size_t entryIndex = 0; entryIndex < snapshot.entries.size(); ++entryIndex)
    {
        const auto& entry = snapshot.entries[entryIndex];
        out << "    {\n";
        out << "      \"nodeType\": \"" << jsonEscaped (entry.nodeType) << "\",\n";
        out << "      \"executionKind\": \"" << jsonEscaped (entry.executionKind) << "\",\n";
        out << "      \"status\": \"" << jsonEscaped (entry.status) << "\",\n";
        out << "      \"children\": [\n";

        for (size_t childIndex = 0; childIndex < entry.children.size(); ++childIndex)
        {
            const auto& child = entry.children[childIndex];
            out << "        {\n";
            out << "          \"cookIndex\": " << child.cookIndex << ",\n";
            out << "          \"childId\": \"" << jsonEscaped (child.childId) << "\",\n";
            out << "          \"nodeType\": \"" << jsonEscaped (child.nodeType) << "\",\n";
            out << "          \"role\": \"" << jsonEscaped (child.role) << "\",\n";
            out << "          \"status\": \"" << jsonEscaped (child.status) << "\",\n";
            out << "          \"reason\": \"" << jsonEscaped (child.reason) << "\",\n";
            out << "          \"inputs\": ";
            appendValueObject (out, child.inputs);
            out << ",\n";
            out << "          \"outputs\": ";
            appendValueObject (out, child.outputs);
            out << "\n";
            out << "        }";

            if (childIndex + 1 < entry.children.size())
                out << ",";

            out << "\n";
        }

        out << "      ]\n";
        out << "    }";

        if (entryIndex + 1 < snapshot.entries.size())
            out << ",";

        out << "\n";
    }

    out << "  ]\n";
    out << "}\n";
    return out.str();
}
}
