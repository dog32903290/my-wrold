#include "RuntimeRegistry.h"

#include "AudioAnalyzerState.h"
#include "CompoundPatch.h"
#include "StorageContract.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iomanip>
#include <map>
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

void appendCompactStringArray (std::ostringstream& out, const std::vector<std::string>& values)
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

void appendRuntimeOpCatalogArray (std::ostringstream& out, const std::vector<RuntimeOpCatalogEntry>& catalog)
{
    out << "[\n";

    for (size_t index = 0; index < catalog.size(); ++index)
    {
        const auto& entry = catalog[index];
        out << "    { \"nodeType\": \"" << jsonEscaped (entry.nodeType)
            << "\", \"runtimeOp\": \"" << jsonEscaped (entry.runtimeOp) << "\" }";

        if (index + 1 < catalog.size())
            out << ",";

        out << "\n";
    }

    out << "  ]";
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

bool hasValueSources (const std::vector<RuntimeOutputValue>& values)
{
    return std::any_of (values.begin(), values.end(), [] (const auto& value) {
        return ! value.source.empty();
    });
}

void appendValueSourceObject (std::ostringstream& out, const std::vector<RuntimeOutputValue>& values)
{
    out << "{";

    bool wroteAny = false;

    for (const auto& value : values)
    {
        if (value.source.empty())
            continue;

        if (wroteAny)
            out << ",";

        out << " \"" << jsonEscaped (value.id) << "\": \"" << jsonEscaped (value.source) << "\"";
        wroteAny = true;
    }

    if (wroteAny)
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

std::vector<RuntimeRegistryEdge> runtimeEdges (const CompoundPatchSpec& compound)
{
    std::vector<RuntimeRegistryEdge> edges;
    edges.reserve (compound.internalEdges.size());

    for (const auto& edge : compound.internalEdges)
        edges.push_back ({ edge.from, edge.to, edge.dataType });

    return edges;
}

std::vector<RuntimeRegistryPublicOutputMapping> runtimePublicOutputMappings (const CompoundPatchSpec& compound)
{
    std::vector<RuntimeRegistryPublicOutputMapping> mappings;
    mappings.reserve (compound.publicOutputs.size());

    for (const auto& port : compound.publicOutputs)
        mappings.push_back ({ port.id, port.mapsTo });

    return mappings;
}

const RuntimeRegistryChild* findRuntimeChild (const RuntimeRegistryEntry& entry, const std::string& childId)
{
    const auto found = std::find_if (entry.children.begin(), entry.children.end(), [&childId] (const auto& child) {
        return child.id == childId;
    });

    return found == entry.children.end() ? nullptr : &(*found);
}

const RuntimeRegistryEdge* findInboundEdge (const RuntimeRegistryEntry& entry,
                                            const std::string& childId,
                                            const std::string& inputPort)
{
    const auto target = childId + "." + inputPort;
    const auto found = std::find_if (entry.internalEdges.begin(), entry.internalEdges.end(), [&target] (const auto& edge) {
        return edge.to == target;
    });

    return found == entry.internalEdges.end() ? nullptr : &(*found);
}

using RuntimeValueBus = std::map<std::string, RuntimeOutputValue>;
using RuntimeSampleBus = std::map<std::string, std::vector<float>>;

RuntimeOutputValue makeInputValue (const std::string& id,
                                   const RuntimeRegistryEntry& entry,
                                   const std::string& childId,
                                   const std::string& inputPort,
                                   const RuntimeValueBus& valueBus)
{
    const auto* edge = findInboundEdge (entry, childId, inputPort);

    if (edge == nullptr)
        return { id, 0.0, {} };

    const auto found = valueBus.find (edge->from);
    const auto value = found == valueBus.end() ? 0.0 : found->second.value;
    return { id, value, edge->from };
}

void publishValue (RuntimeValueBus& valueBus,
                   std::vector<RuntimeOutputValue>& outputs,
                   const std::string& childId,
                   const std::string& portId,
                   double value)
{
    const RuntimeOutputValue output { portId, value, {} };
    outputs.push_back (output);
    valueBus[childId + "." + portId] = output;
}

bool hasValue (const RuntimeValueBus& valueBus, const std::string& key)
{
    return valueBus.find (key) != valueBus.end();
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

struct SyntheticRuntimeOpContext
{
    const RuntimeRegistryEntry& entry;
    const RuntimeRegistryChild& child;
    const RuntimeSyntheticAudioInput& input;
    size_t sampleCount = 0;
    RuntimeValueBus& valueBus;
    RuntimeSampleBus& sampleBus;
};

using SyntheticRuntimeOpFunction = void (*) (SyntheticRuntimeOpContext&, RuntimeChildExecutionStatus&);

struct SyntheticRuntimeOpDefinition
{
    const char* nodeType = "";
    const char* id = "";
    SyntheticRuntimeOpFunction execute = nullptr;
};

void runSyntheticAudioInputOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    childStatus.status = "computed";
    childStatus.reason = "accepted synthetic audio channels";
    publishValue (context.valueBus,
                  childStatus.outputs,
                  context.child.id,
                  "channels",
                  static_cast<double> (context.input.channels.size()));
    publishValue (context.valueBus,
                  childStatus.outputs,
                  context.child.id,
                  "sampleCount",
                  static_cast<double> (context.sampleCount));
}

void runSyntheticMonoMixOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto audioInput = makeInputValue ("input", context.entry, context.child.id, "input", context.valueBus);
    childStatus.inputs = { audioInput };

    if (hasValue (context.valueBus, audioInput.source))
    {
        auto monoMix = mixToMono (context.input.channels);
        context.sampleBus[context.child.id + ".mono"] = monoMix.samples;
        childStatus.status = "computed";
        childStatus.reason = "averaged synthetic channels into mono samples";
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "sampleCount",
                      static_cast<double> (monoMix.samples.size()));
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "rms", monoMix.rms);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "peak", monoMix.peak);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "mono", monoMix.rms);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for audio.input output";
    }
}

void runSyntheticAnalyzerRmsOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto monoInput = makeInputValue ("input", context.entry, context.child.id, "input", context.valueBus);
    childStatus.inputs = { monoInput };
    const auto sampleSource = context.sampleBus.find (monoInput.source);

    if (sampleSource != context.sampleBus.end())
    {
        const auto& monoSamples = sampleSource->second;
        const float* channels[] { monoSamples.data() };
        AudioAnalyzerState analyzer;
        analyzer.processBlock (channels, 1, static_cast<int> (monoSamples.size()), 1.0f);
        const auto analyzerSnapshot = analyzer.getSnapshot();

        childStatus.status = "computed";
        childStatus.reason = "computed rms/peak from audio.mono_mix output";
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "rms",
                      static_cast<double> (analyzerSnapshot.rms));
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "peak",
                      static_cast<double> (analyzerSnapshot.peak));
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for audio.mono_mix output";
    }
}

void runSyntheticAnalysisGainOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto measuredInput = makeInputValue ("input", context.entry, context.child.id, "input", context.valueBus);
    childStatus.inputs = {
        measuredInput,
        { "gain", static_cast<double> (context.input.analysisGain) }
    };

    if (hasValue (context.valueBus, measuredInput.source))
    {
        const auto calibrated = measuredInput.value * static_cast<double> (context.input.analysisGain);
        childStatus.status = "computed";
        childStatus.reason = "calibrated analyzer.rms output with analysis gain";
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "out", calibrated);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for analyzer.rms output";
    }
}

void runSyntheticPreGateOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    constexpr double gateThreshold = 0.0001;
    const auto calibratedInput = makeInputValue ("input", context.entry, context.child.id, "input", context.valueBus);
    childStatus.inputs = {
        calibratedInput,
        { "threshold", gateThreshold }
    };

    if (hasValue (context.valueBus, calibratedInput.source))
    {
        const auto gateValue = calibratedInput.value > gateThreshold ? 1.0 : 0.0;
        const auto confidenceValue = gateValue;
        const auto gateOutput = calibratedInput.value * gateValue;
        childStatus.status = "computed";
        childStatus.reason = "gated calibrated loudness with fixed first-proof threshold";
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "out", gateOutput);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "gate", gateValue);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "confidence", confidenceValue);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for analyzer.analysis_gain output";
    }
}

void runSyntheticSmootherOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto gatedInput = makeInputValue ("input", context.entry, context.child.id, "input", context.valueBus);
    childStatus.inputs = { gatedInput };

    if (hasValue (context.valueBus, gatedInput.source))
    {
        childStatus.status = "computed";
        childStatus.reason = "first-proof pass-through smoother";
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "out", gatedInput.value);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for analyzer.pre_gate output";
    }
}

void runSyntheticLoudnessOutOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto smoothedInput = makeInputValue ("input", context.entry, context.child.id, "input", context.valueBus);
    const auto rmsInput = RuntimeOutputValue { "rms",
                                               hasValue (context.valueBus, "rms.rms")
                                                   ? context.valueBus.at ("rms.rms").value
                                                   : 0.0,
                                               "rms.rms" };
    const auto peakInput = makeInputValue ("peak", context.entry, context.child.id, "peak", context.valueBus);
    const auto gateInput = RuntimeOutputValue { "gate",
                                                hasValue (context.valueBus, "pre_gate.gate")
                                                    ? context.valueBus.at ("pre_gate.gate").value
                                                    : 0.0,
                                                "pre_gate.gate" };
    const auto confidenceInput = makeInputValue ("confidence",
                                                 context.entry,
                                                 context.child.id,
                                                 "confidence",
                                                 context.valueBus);
    childStatus.inputs = {
        smoothedInput,
        rmsInput,
        peakInput,
        gateInput,
        confidenceInput
    };

    if (hasValue (context.valueBus, smoothedInput.source)
        && hasValue (context.valueBus, rmsInput.source)
        && hasValue (context.valueBus, peakInput.source)
        && hasValue (context.valueBus, gateInput.source)
        && hasValue (context.valueBus, confidenceInput.source))
    {
        childStatus.status = "computed";
        childStatus.reason = "published loaded compound public outputs";
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "out", smoothedInput.value);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "rms", rmsInput.value);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "peak", peakInput.value);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "gate", gateInput.value);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "confidence",
                      confidenceInput.value);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for signal.smoother output";
    }
}

const std::vector<SyntheticRuntimeOpDefinition>& syntheticRuntimeOps()
{
    static const std::vector<SyntheticRuntimeOpDefinition> ops {
        { "audio.input", "synthetic.audio.input", runSyntheticAudioInputOp },
        { "audio.mono_mix", "synthetic.audio.mono_mix", runSyntheticMonoMixOp },
        { "analyzer.rms", "synthetic.analyzer.rms", runSyntheticAnalyzerRmsOp },
        { "analyzer.analysis_gain", "synthetic.analyzer.analysis_gain", runSyntheticAnalysisGainOp },
        { "analyzer.pre_gate", "synthetic.analyzer.pre_gate", runSyntheticPreGateOp },
        { "signal.smoother", "synthetic.signal.smoother", runSyntheticSmootherOp },
        { "analyzer.loudness_out", "synthetic.analyzer.loudness_out", runSyntheticLoudnessOutOp }
    };

    return ops;
}

const SyntheticRuntimeOpDefinition* findSyntheticRuntimeOp (const std::string& nodeType)
{
    const auto& ops = syntheticRuntimeOps();
    const auto found = std::find_if (ops.begin(), ops.end(), [&nodeType] (const auto& op) {
        return op.nodeType == nodeType;
    });

    return found == ops.end() ? nullptr : &(*found);
}

std::string makeMissingRuntimeOpReason (const std::string& nodeType)
{
    return "missing RuntimeOp for " + nodeType;
}

std::string makeMissingRuntimeOpError (const std::string& phase,
                                       const RuntimeRegistryEntry& entry,
                                       const RuntimeRegistryChild& child)
{
    return phase + " missing RuntimeOp for " + entry.nodeType + ":" + child.id + " (" + child.nodeType + ")";
}

std::vector<std::string> missingNodeTypesFor (const RuntimeEntryCoverageStatus& entry)
{
    std::vector<std::string> missing;

    for (const auto& child : entry.children)
    {
        if (child.status != "missing-runtime-op")
            continue;

        if (std::find (missing.begin(), missing.end(), child.nodeType) == missing.end())
            missing.push_back (child.nodeType);
    }

    return missing;
}

std::string joinedNodeTypes (const std::vector<std::string>& nodeTypes)
{
    std::ostringstream text;

    for (size_t index = 0; index < nodeTypes.size(); ++index)
    {
        if (index != 0)
            text << ", ";

        text << nodeTypes[index];
    }

    return text.str();
}

RuntimeRegistryEntry makeRuntimeRegistryEntry (const ModulePackageManifest& module, const CompoundPatchSpec& compound)
{
    RuntimeRegistryEntry entry;
    entry.nodeType = fallback (module.nodeType, compound.type);
    entry.displayName = fallback (module.title, compound.displayName);
    entry.runtimeDomain = fallback (module.runtimeDomain, "graph");
    entry.executionKind = "compound.patch";
    entry.previewPolicy = fallback (module.previewPolicy, "none");
    entry.children = runtimeChildren (compound);
    entry.childCount = compound.children.size();
    entry.internalEdgeCount = compound.internalEdges.size();
    entry.internalEdges = runtimeEdges (compound);
    entry.publicInputs = portIds (compound.publicInputs);
    entry.publicOutputs = portIds (compound.publicOutputs);
    entry.publicOutputMappings = runtimePublicOutputMappings (compound);
    entry.cookOrder = makeCompoundCookOrder (compound);
    return entry;
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

std::vector<RuntimeOpCatalogEntry> makeRuntimeOpCatalog()
{
    std::vector<RuntimeOpCatalogEntry> catalog;
    catalog.reserve (syntheticRuntimeOps().size());

    for (const auto& op : syntheticRuntimeOps())
        catalog.push_back ({ op.nodeType, op.id });

    return catalog;
}

RuntimeOpCoverageResult inspectRuntimeOpCoverage (const RuntimeRegistry& registry)
{
    RuntimeOpCoverageSnapshot snapshot;
    snapshot.version = registry.version;
    snapshot.catalog = makeRuntimeOpCatalog();

    bool ok = true;
    std::string firstError;

    for (const auto& entry : registry.entries)
    {
        RuntimeEntryCoverageStatus entryStatus;
        entryStatus.nodeType = entry.nodeType;
        entryStatus.executionKind = entry.executionKind;
        entryStatus.status = "runtime-op-covered";
        entryStatus.children.reserve (entry.cookOrder.size());

        for (size_t cookIndex = 0; cookIndex < entry.cookOrder.size(); ++cookIndex)
        {
            const auto& childId = entry.cookOrder[cookIndex];
            const auto* child = findRuntimeChild (entry, childId);

            if (child == nullptr)
                return { false, snapshot, "coverage missing child metadata for " + entry.nodeType + ":" + childId };

            const auto* runtimeOp = findSyntheticRuntimeOp (child->nodeType);

            if (runtimeOp == nullptr)
            {
                ok = false;
                entryStatus.status = "missing-runtime-op";
                ++entryStatus.missingChildCount;
                ++snapshot.missingChildCount;

                if (firstError.empty())
                    firstError = makeMissingRuntimeOpError ("coverage", entry, *child);

                entryStatus.children.push_back ({ cookIndex,
                                                  child->id,
                                                  child->nodeType,
                                                  child->role,
                                                  {},
                                                  "missing-runtime-op",
                                                  makeMissingRuntimeOpReason (child->nodeType) });
                continue;
            }

            ++entryStatus.supportedChildCount;
            ++snapshot.supportedChildCount;
            entryStatus.children.push_back ({ cookIndex,
                                              child->id,
                                              child->nodeType,
                                              child->role,
                                              runtimeOp->id,
                                              "supported-runtime-op",
                                              "RuntimeOp is registered; execution not run" });
        }

        if (entryStatus.children.size() != entry.childCount)
            return { false, snapshot, "coverage child count mismatch for " + entry.nodeType };

        snapshot.entries.push_back (entryStatus);
    }

    return { ok, snapshot, ok ? std::string {} : firstError };
}

std::vector<RuntimeOpModuleDiagnostic> makeRuntimeOpModuleDiagnostics (const RuntimeOpCoverageSnapshot& snapshot)
{
    std::vector<RuntimeOpModuleDiagnostic> diagnostics;
    diagnostics.reserve (snapshot.entries.size());

    for (const auto& entry : snapshot.entries)
    {
        RuntimeOpModuleDiagnostic diagnostic;
        diagnostic.nodeType = entry.nodeType;
        diagnostic.supportedChildCount = entry.supportedChildCount;
        diagnostic.missingChildCount = entry.missingChildCount;
        diagnostic.missingNodeTypes = missingNodeTypesFor (entry);

        if (entry.missingChildCount == 0)
        {
            diagnostic.status = "runtime-op-ready";
            diagnostic.browserLabel = "runtime ready";
            diagnostic.inspectorDetail = std::to_string (entry.supportedChildCount)
                                       + " RuntimeOps registered; execution not run";
            diagnostic.creationStatus = "create-enabled";
            diagnostic.creationLabel = "create";
        }
        else
        {
            diagnostic.status = "missing-runtime-op";
            diagnostic.browserLabel = "missing RuntimeOp";
            diagnostic.inspectorDetail = "missing RuntimeOp: " + joinedNodeTypes (diagnostic.missingNodeTypes);
            diagnostic.creationStatus = "create-blocked";
            diagnostic.creationLabel = "blocked";
            diagnostic.creationBlockReason = diagnostic.inspectorDetail;
        }

        diagnostics.push_back (std::move (diagnostic));
    }

    return diagnostics;
}

bool runtimeOpDiagnosticAllowsCreation (const RuntimeOpModuleDiagnostic& diagnostic)
{
    return diagnostic.creationStatus != "create-blocked";
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

            const auto* runtimeOp = findSyntheticRuntimeOp (child->nodeType);

            if (runtimeOp == nullptr)
            {
                entryStatus.status = "missing-runtime-op";
                entryStatus.children.push_back ({ cookIndex,
                                                 child->id,
                                                 child->nodeType,
                                                 child->role,
                                                 {},
                                                 "missing-runtime-op",
                                                 makeMissingRuntimeOpReason (child->nodeType) });
                snapshot.entries.push_back (entryStatus);
                return { false, snapshot, makeMissingRuntimeOpError ("dry-run", entry, *child) };
            }

            entryStatus.children.push_back ({ cookIndex,
                                             child->id,
                                             child->nodeType,
                                             child->role,
                                             runtimeOp->id,
                                             "dry-run-ready",
                                             "validated child order and RuntimeOp coverage; RuntimeOp not executed" });
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

        RuntimeValueBus valueBus;
        RuntimeSampleBus sampleBus;

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
                {},
                "not-executed",
                "RuntimeOp not implemented for " + child->nodeType,
                {},
                {}
            };

            const auto* runtimeOp = findSyntheticRuntimeOp (child->nodeType);

            if (runtimeOp == nullptr)
            {
                childStatus.status = "missing-runtime-op";
                childStatus.reason = makeMissingRuntimeOpReason (child->nodeType);
                entryStatus.status = "missing-runtime-op";
                entryStatus.children.push_back (childStatus);
                snapshot.entries.push_back (entryStatus);
                return { false, snapshot, makeMissingRuntimeOpError ("execution", entry, *child) };
            }

            childStatus.runtimeOp = runtimeOp->id;
            SyntheticRuntimeOpContext context { entry, *child, input, sampleCount, valueBus, sampleBus };
            runtimeOp->execute (context, childStatus);

            entryStatus.children.push_back (childStatus);
        }

        for (const auto& mapping : entry.publicOutputMappings)
        {
            const auto found = valueBus.find (mapping.mapsTo);

            if (found != valueBus.end())
                entryStatus.publicOutputs.push_back ({ mapping.id, found->second.value, mapping.mapsTo });
        }

        if (! entryStatus.publicOutputs.empty()
            && entryStatus.publicOutputs.size() == entry.publicOutputMappings.size()
            && std::all_of (entryStatus.children.begin(), entryStatus.children.end(), [] (const auto& child) {
                return child.status == "computed";
            }))
        {
            entryStatus.status = "computed";
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
        out << "      \"internalEdges\": [\n";

        for (size_t edgeIndex = 0; edgeIndex < entry.internalEdges.size(); ++edgeIndex)
        {
            const auto& edge = entry.internalEdges[edgeIndex];
            out << "        { \"from\": \"" << jsonEscaped (edge.from)
                << "\", \"to\": \"" << jsonEscaped (edge.to)
                << "\", \"dataType\": \"" << jsonEscaped (edge.dataType) << "\" }";

            if (edgeIndex + 1 < entry.internalEdges.size())
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
        out << "      \"publicOutputMappings\": [\n";

        for (size_t mappingIndex = 0; mappingIndex < entry.publicOutputMappings.size(); ++mappingIndex)
        {
            const auto& mapping = entry.publicOutputMappings[mappingIndex];
            out << "        { \"id\": \"" << jsonEscaped (mapping.id)
                << "\", \"mapsTo\": \"" << jsonEscaped (mapping.mapsTo) << "\" }";

            if (mappingIndex + 1 < entry.publicOutputMappings.size())
                out << ",";

            out << "\n";
        }

        out << "      ],\n";
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

std::string makeRuntimeOpCatalogJson (const std::vector<RuntimeOpCatalogEntry>& catalog)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"runtimeOpCatalog\",\n";
    out << "  \"version\": 1,\n";
    out << "  \"entries\": ";
    appendRuntimeOpCatalogArray (out, catalog);
    out << "\n";
    out << "}\n";
    return out.str();
}

std::string makeRuntimeOpCoverageJson (const RuntimeOpCoverageSnapshot& snapshot)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"runtimeOpCoverage\",\n";
    out << "  \"version\": " << snapshot.version << ",\n";
    out << "  \"mode\": \"" << jsonEscaped (snapshot.mode) << "\",\n";
    out << "  \"supportedChildCount\": " << snapshot.supportedChildCount << ",\n";
    out << "  \"missingChildCount\": " << snapshot.missingChildCount << ",\n";
    out << "  \"catalog\": ";
    appendRuntimeOpCatalogArray (out, snapshot.catalog);
    out << ",\n";
    out << "  \"entries\": [\n";

    for (size_t entryIndex = 0; entryIndex < snapshot.entries.size(); ++entryIndex)
    {
        const auto& entry = snapshot.entries[entryIndex];
        out << "    {\n";
        out << "      \"nodeType\": \"" << jsonEscaped (entry.nodeType) << "\",\n";
        out << "      \"executionKind\": \"" << jsonEscaped (entry.executionKind) << "\",\n";
        out << "      \"status\": \"" << jsonEscaped (entry.status) << "\",\n";
        out << "      \"supportedChildCount\": " << entry.supportedChildCount << ",\n";
        out << "      \"missingChildCount\": " << entry.missingChildCount << ",\n";
        out << "      \"children\": [\n";

        for (size_t childIndex = 0; childIndex < entry.children.size(); ++childIndex)
        {
            const auto& child = entry.children[childIndex];
            out << "        {\n";
            out << "          \"cookIndex\": " << child.cookIndex << ",\n";
            out << "          \"childId\": \"" << jsonEscaped (child.childId) << "\",\n";
            out << "          \"nodeType\": \"" << jsonEscaped (child.nodeType) << "\",\n";
            out << "          \"role\": \"" << jsonEscaped (child.role) << "\",\n";
            out << "          \"runtimeOp\": \"" << jsonEscaped (child.runtimeOp) << "\",\n";
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

std::string makeRuntimeOpModuleDiagnosticsJson (const std::vector<RuntimeOpModuleDiagnostic>& diagnostics)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"runtimeOpModuleDiagnostics\",\n";
    out << "  \"version\": 1,\n";
    out << "  \"visibleIn\": [\"browser\", \"inspector\", \"leftRail\"],\n";
    out << "  \"entries\": [\n";

    for (size_t index = 0; index < diagnostics.size(); ++index)
    {
        const auto& diagnostic = diagnostics[index];
        out << "    {\n";
        out << "      \"nodeType\": \"" << jsonEscaped (diagnostic.nodeType) << "\",\n";
        out << "      \"status\": \"" << jsonEscaped (diagnostic.status) << "\",\n";
        out << "      \"browserLabel\": \"" << jsonEscaped (diagnostic.browserLabel) << "\",\n";
        out << "      \"inspectorDetail\": \"" << jsonEscaped (diagnostic.inspectorDetail) << "\",\n";
        out << "      \"creationStatus\": \"" << jsonEscaped (diagnostic.creationStatus) << "\",\n";
        out << "      \"creationLabel\": \"" << jsonEscaped (diagnostic.creationLabel) << "\",\n";
        out << "      \"creationBlockReason\": \"" << jsonEscaped (diagnostic.creationBlockReason) << "\",\n";
        out << "      \"supportedChildCount\": " << diagnostic.supportedChildCount << ",\n";
        out << "      \"missingChildCount\": " << diagnostic.missingChildCount << ",\n";
        out << "      \"missingNodeTypes\": ";
        appendCompactStringArray (out, diagnostic.missingNodeTypes);
        out << "\n";
        out << "    }";

        if (index + 1 < diagnostics.size())
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
            out << "          \"runtimeOp\": \"" << jsonEscaped (child.runtimeOp) << "\",\n";
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
        out << "      \"publicOutputs\": ";
        appendValueObject (out, entry.publicOutputs);
        out << ",\n";
        out << "      \"publicOutputSources\": ";
        appendValueSourceObject (out, entry.publicOutputs);
        out << ",\n";
        out << "      \"children\": [\n";

        for (size_t childIndex = 0; childIndex < entry.children.size(); ++childIndex)
        {
            const auto& child = entry.children[childIndex];
            out << "        {\n";
            out << "          \"cookIndex\": " << child.cookIndex << ",\n";
            out << "          \"childId\": \"" << jsonEscaped (child.childId) << "\",\n";
            out << "          \"nodeType\": \"" << jsonEscaped (child.nodeType) << "\",\n";
            out << "          \"role\": \"" << jsonEscaped (child.role) << "\",\n";
            out << "          \"runtimeOp\": \"" << jsonEscaped (child.runtimeOp) << "\",\n";
            out << "          \"status\": \"" << jsonEscaped (child.status) << "\",\n";
            out << "          \"reason\": \"" << jsonEscaped (child.reason) << "\",\n";
            out << "          \"inputs\": ";
            appendValueObject (out, child.inputs);
            out << ",\n";
            out << "          \"inputSources\": ";
            appendValueSourceObject (out, child.inputs);
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
