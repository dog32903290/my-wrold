#include "RuntimeRegistry.h"

#include "AudioAnalyzerState.h"
#include "CompoundPatch.h"
#include "JsonWriter.h"
#include "PathResolution.h"
#include "StorageContract.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <map>
#include <sstream>

namespace myworld
{
namespace
{
void appendRuntimeOpCatalogArray (std::ostringstream& out, const std::vector<RuntimeOpCatalogEntry>& catalog)
{
    out << "[\n";

    for (size_t index = 0; index < catalog.size(); ++index)
    {
        const auto& entry = catalog[index];
        out << "    { \"nodeType\": " << jsonQuoted (entry.nodeType)
            << ", \"runtimeOp\": " << jsonQuoted (entry.runtimeOp) << " }";

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

        out << " " << jsonQuoted (values[index].id) << ": " << values[index].value;
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

        out << " " << jsonQuoted (value.id) << ": " << jsonQuoted (value.source);
        wroteAny = true;
    }

    if (wroteAny)
        out << " ";

    out << "}";
}

const RuntimeOutputValue* findOutputValue (const std::vector<RuntimeOutputValue>& values, const std::string& id)
{
    const auto found = std::find_if (values.begin(), values.end(), [&id] (const auto& value) {
        return value.id == id;
    });

    return found == values.end() ? nullptr : &*found;
}

bool hasLoudnessPublicOutputs (const RuntimeEntryExecutionStatus& entry)
{
    return findOutputValue (entry.publicOutputs, "out") != nullptr
           && findOutputValue (entry.publicOutputs, "rms") != nullptr
           && findOutputValue (entry.publicOutputs, "peak") != nullptr
           && findOutputValue (entry.publicOutputs, "gate") != nullptr
           && findOutputValue (entry.publicOutputs, "confidence") != nullptr;
}

std::vector<RuntimeOutputValue> orderedLoudnessPublicOutputs (const std::vector<RuntimeOutputValue>& values)
{
    std::vector<RuntimeOutputValue> ordered;
    ordered.reserve (5);

    for (const auto& id : { "out", "rms", "peak", "gate", "confidence" })
        if (const auto* value = findOutputValue (values, id))
            ordered.push_back (*value);

    return ordered;
}

std::string fallback (const std::string& value, const std::string& fallbackValue)
{
    return value.empty() ? fallbackValue : value;
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

void runSyntheticRawEnergyOutOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto rmsInput = makeInputValue ("rms", context.entry, context.child.id, "rms", context.valueBus);
    const auto peakInput = makeInputValue ("peak", context.entry, context.child.id, "peak", context.valueBus);
    const auto sampleCountInput = makeInputValue ("sampleCount",
                                                  context.entry,
                                                  context.child.id,
                                                  "sampleCount",
                                                  context.valueBus);
    childStatus.inputs = {
        rmsInput,
        peakInput,
        sampleCountInput
    };

    if (hasValue (context.valueBus, rmsInput.source)
        && hasValue (context.valueBus, peakInput.source)
        && hasValue (context.valueBus, sampleCountInput.source))
    {
        childStatus.status = "computed";
        childStatus.reason = "published raw energy facts before detector shaping";
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "rms", rmsInput.value);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "peak", peakInput.value);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "sampleCount",
                      sampleCountInput.value);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for analyzer.rms and audio.mono_mix raw facts";
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
        { "analyzer.raw_energy_out", "synthetic.analyzer.raw_energy_out", runSyntheticRawEnergyOutOp },
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
    const auto libraryResolution = resolvePathNearWithReport ({}, libraryPath);
    if (! libraryResolution.found)
        return { false, {}, describePathResolutionFailure ("module library", libraryResolution) };

    const auto library = loadModuleLibraryManifest (libraryResolution.resolvedPath);
    if (! library.ok)
        return { false, {}, library.error };

    RuntimeRegistry registry;

    for (const auto& modulePath : library.manifest.modulePackages)
    {
        const auto moduleResolution = resolvePathNearWithReport (libraryResolution.resolvedPath, modulePath);
        if (! moduleResolution.found)
            return { false, {}, describePathResolutionFailure ("module manifest", moduleResolution) };

        const auto module = loadModulePackageManifest (moduleResolution.resolvedPath);
        if (! module.ok)
            return { false, {}, module.error };

        const auto compoundResolution = resolvePathNearWithReport (moduleResolution.resolvedPath, module.manifest.patchPath);
        if (! compoundResolution.found)
            return { false, {}, describePathResolutionFailure ("compound patch", compoundResolution) };

        const auto compound = loadCompoundPatchSpec (compoundResolution.resolvedPath);
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

RuntimeSyntheticAudioInput makeRuntimeSyntheticAudioInputFromAnalyzerSnapshot (const AudioAnalyzerSnapshot& snapshot,
                                                                               const size_t sampleCount)
{
    RuntimeSyntheticAudioInput input;
    input.analysisGain = 1.0f;

    const auto safeSampleCount = std::max<size_t> (1, sampleCount);
    input.channels.emplace_back (safeSampleCount, 0.0f);

    const auto amplitude = std::isfinite (snapshot.rms) ? std::abs (snapshot.rms) : 0.0f;

    for (size_t index = 0; index < safeSampleCount; ++index)
        input.channels.back()[index] = (index % 2 == 0) ? amplitude : -amplitude;

    return input;
}

static LoudnessRuntimeBridgeSnapshot makeLoudnessRuntimeFallbackSnapshot (const AudioAnalyzerSnapshot& fallbackSnapshot)
{
    return {
        false,
        "direct-analyzer-fallback",
        {
            { "out", static_cast<double> (fallbackSnapshot.loudness), "audioInputAnalyzer.loudness" },
            { "rms", static_cast<double> (fallbackSnapshot.rms), "audioInputAnalyzer.rms" },
            { "peak", static_cast<double> (fallbackSnapshot.peak), "audioInputAnalyzer.peak" },
            { "gate", static_cast<double> (fallbackSnapshot.gate), "audioInputAnalyzer.gate" },
            { "confidence", static_cast<double> (fallbackSnapshot.confidence), "audioInputAnalyzer.confidence" }
        },
        fallbackSnapshot
    };
}

LoudnessRuntimeBridgeSnapshot makeLoudnessRuntimeBridgeSnapshot (const RuntimeExecutionSnapshot& runtimeSnapshot,
                                                                 const AudioAnalyzerSnapshot& fallbackSnapshot)
{
    for (const auto& entry : runtimeSnapshot.entries)
    {
        if (entry.status != "computed" || ! hasLoudnessPublicOutputs (entry))
            continue;

        auto bridge = makeLoudnessRuntimeFallbackSnapshot (fallbackSnapshot);
        bridge.usesLoadedRuntimeOutputs = true;
        bridge.sourceMode = "loaded-runtime-publicOutputs";
        bridge.publicOutputs = orderedLoudnessPublicOutputs (entry.publicOutputs);

        const auto* out = findOutputValue (bridge.publicOutputs, "out");
        const auto* rms = findOutputValue (bridge.publicOutputs, "rms");
        const auto* peak = findOutputValue (bridge.publicOutputs, "peak");
        const auto* gate = findOutputValue (bridge.publicOutputs, "gate");
        const auto* confidence = findOutputValue (bridge.publicOutputs, "confidence");

        bridge.analyzer.loudness = out == nullptr ? fallbackSnapshot.loudness : static_cast<float> (out->value);
        bridge.analyzer.rms = rms == nullptr ? fallbackSnapshot.rms : static_cast<float> (rms->value);
        bridge.analyzer.peak = peak == nullptr ? fallbackSnapshot.peak : static_cast<float> (peak->value);
        bridge.analyzer.gate = gate == nullptr ? fallbackSnapshot.gate : static_cast<float> (gate->value);
        bridge.analyzer.confidence = confidence == nullptr ? fallbackSnapshot.confidence : static_cast<float> (confidence->value);
        bridge.analyzer.active = bridge.analyzer.gate > 0.0f;
        bridge.analyzer.sampleCounter = fallbackSnapshot.sampleCounter;
        return bridge;
    }

    return makeLoudnessRuntimeFallbackSnapshot (fallbackSnapshot);
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
        out << "      \"nodeType\": " << jsonQuoted (entry.nodeType) << ",\n";
        out << "      \"displayName\": " << jsonQuoted (entry.displayName) << ",\n";
        out << "      \"runtimeDomain\": " << jsonQuoted (entry.runtimeDomain) << ",\n";
        out << "      \"executionKind\": " << jsonQuoted (entry.executionKind) << ",\n";
        out << "      \"previewPolicy\": " << jsonQuoted (entry.previewPolicy) << ",\n";
        out << "      \"childCount\": " << entry.childCount << ",\n";
        out << "      \"internalEdgeCount\": " << entry.internalEdgeCount << ",\n";
        out << "      \"children\": [\n";

        for (size_t childIndex = 0; childIndex < entry.children.size(); ++childIndex)
        {
            const auto& child = entry.children[childIndex];
            out << "        { \"id\": " << jsonQuoted (child.id)
                << ", \"nodeType\": " << jsonQuoted (child.nodeType)
                << ", \"role\": " << jsonQuoted (child.role) << " }";

            if (childIndex + 1 < entry.children.size())
                out << ",";

            out << "\n";
        }

        out << "      ],\n";
        out << "      \"internalEdges\": [\n";

        for (size_t edgeIndex = 0; edgeIndex < entry.internalEdges.size(); ++edgeIndex)
        {
            const auto& edge = entry.internalEdges[edgeIndex];
            out << "        { \"from\": " << jsonQuoted (edge.from)
                << ", \"to\": " << jsonQuoted (edge.to)
                << ", \"dataType\": " << jsonQuoted (edge.dataType) << " }";

            if (edgeIndex + 1 < entry.internalEdges.size())
                out << ",";

            out << "\n";
        }

        out << "      ],\n";
        out << "      \"publicInputs\": ";
        appendJsonStringArray (out, entry.publicInputs);
        out << ",\n";
        out << "      \"publicOutputs\": ";
        appendJsonStringArray (out, entry.publicOutputs);
        out << ",\n";
        out << "      \"publicOutputMappings\": [\n";

        for (size_t mappingIndex = 0; mappingIndex < entry.publicOutputMappings.size(); ++mappingIndex)
        {
            const auto& mapping = entry.publicOutputMappings[mappingIndex];
            out << "        { \"id\": " << jsonQuoted (mapping.id)
                << ", \"mapsTo\": " << jsonQuoted (mapping.mapsTo) << " }";

            if (mappingIndex + 1 < entry.publicOutputMappings.size())
                out << ",";

            out << "\n";
        }

        out << "      ],\n";
        out << "      \"cookOrder\": ";
        appendJsonStringArray (out, entry.cookOrder);
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
    out << "  \"mode\": " << jsonQuoted (snapshot.mode) << ",\n";
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
        out << "      \"nodeType\": " << jsonQuoted (entry.nodeType) << ",\n";
        out << "      \"executionKind\": " << jsonQuoted (entry.executionKind) << ",\n";
        out << "      \"status\": " << jsonQuoted (entry.status) << ",\n";
        out << "      \"supportedChildCount\": " << entry.supportedChildCount << ",\n";
        out << "      \"missingChildCount\": " << entry.missingChildCount << ",\n";
        out << "      \"children\": [\n";

        for (size_t childIndex = 0; childIndex < entry.children.size(); ++childIndex)
        {
            const auto& child = entry.children[childIndex];
            out << "        {\n";
            out << "          \"cookIndex\": " << child.cookIndex << ",\n";
            out << "          \"childId\": " << jsonQuoted (child.childId) << ",\n";
            out << "          \"nodeType\": " << jsonQuoted (child.nodeType) << ",\n";
            out << "          \"role\": " << jsonQuoted (child.role) << ",\n";
            out << "          \"runtimeOp\": " << jsonQuoted (child.runtimeOp) << ",\n";
            out << "          \"status\": " << jsonQuoted (child.status) << ",\n";
            out << "          \"reason\": " << jsonQuoted (child.reason) << "\n";
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
        out << "      \"nodeType\": " << jsonQuoted (diagnostic.nodeType) << ",\n";
        out << "      \"status\": " << jsonQuoted (diagnostic.status) << ",\n";
        out << "      \"browserLabel\": " << jsonQuoted (diagnostic.browserLabel) << ",\n";
        out << "      \"inspectorDetail\": " << jsonQuoted (diagnostic.inspectorDetail) << ",\n";
        out << "      \"creationStatus\": " << jsonQuoted (diagnostic.creationStatus) << ",\n";
        out << "      \"creationLabel\": " << jsonQuoted (diagnostic.creationLabel) << ",\n";
        out << "      \"creationBlockReason\": " << jsonQuoted (diagnostic.creationBlockReason) << ",\n";
        out << "      \"supportedChildCount\": " << diagnostic.supportedChildCount << ",\n";
        out << "      \"missingChildCount\": " << diagnostic.missingChildCount << ",\n";
        out << "      \"missingNodeTypes\": ";
        appendJsonStringArray (out, diagnostic.missingNodeTypes);
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
    out << "  \"mode\": " << jsonQuoted (snapshot.mode) << ",\n";
    out << "  \"entries\": [\n";

    for (size_t entryIndex = 0; entryIndex < snapshot.entries.size(); ++entryIndex)
    {
        const auto& entry = snapshot.entries[entryIndex];
        out << "    {\n";
        out << "      \"nodeType\": " << jsonQuoted (entry.nodeType) << ",\n";
        out << "      \"executionKind\": " << jsonQuoted (entry.executionKind) << ",\n";
        out << "      \"status\": " << jsonQuoted (entry.status) << ",\n";
        out << "      \"children\": [\n";

        for (size_t childIndex = 0; childIndex < entry.children.size(); ++childIndex)
        {
            const auto& child = entry.children[childIndex];
            out << "        {\n";
            out << "          \"cookIndex\": " << child.cookIndex << ",\n";
            out << "          \"childId\": " << jsonQuoted (child.childId) << ",\n";
            out << "          \"nodeType\": " << jsonQuoted (child.nodeType) << ",\n";
            out << "          \"role\": " << jsonQuoted (child.role) << ",\n";
            out << "          \"runtimeOp\": " << jsonQuoted (child.runtimeOp) << ",\n";
            out << "          \"status\": " << jsonQuoted (child.status) << ",\n";
            out << "          \"reason\": " << jsonQuoted (child.reason) << "\n";
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
    out << "  \"mode\": " << jsonQuoted (snapshot.mode) << ",\n";
    out << "  \"entries\": [\n";

    for (size_t entryIndex = 0; entryIndex < snapshot.entries.size(); ++entryIndex)
    {
        const auto& entry = snapshot.entries[entryIndex];
        out << "    {\n";
        out << "      \"nodeType\": " << jsonQuoted (entry.nodeType) << ",\n";
        out << "      \"executionKind\": " << jsonQuoted (entry.executionKind) << ",\n";
        out << "      \"status\": " << jsonQuoted (entry.status) << ",\n";
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
            out << "          \"childId\": " << jsonQuoted (child.childId) << ",\n";
            out << "          \"nodeType\": " << jsonQuoted (child.nodeType) << ",\n";
            out << "          \"role\": " << jsonQuoted (child.role) << ",\n";
            out << "          \"runtimeOp\": " << jsonQuoted (child.runtimeOp) << ",\n";
            out << "          \"status\": " << jsonQuoted (child.status) << ",\n";
            out << "          \"reason\": " << jsonQuoted (child.reason) << ",\n";
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

std::string makeLoudnessRuntimeBridgeJson (const LoudnessRuntimeBridgeSnapshot& snapshot)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"loudnessRuntimeBridge\",\n";
    out << "  \"sourceMode\": " << jsonQuoted (snapshot.sourceMode) << ",\n";
    out << "  \"usesLoadedRuntimeOutputs\": " << (snapshot.usesLoadedRuntimeOutputs ? "true" : "false") << ",\n";
    out << "  \"publicOutputs\": ";
    appendValueObject (out, snapshot.publicOutputs);
    out << ",\n";
    out << "  \"publicOutputSources\": ";
    appendValueSourceObject (out, snapshot.publicOutputs);
    out << ",\n";
    out << "  \"analyzer\": {\n";
    out << "    \"rms\": " << snapshot.analyzer.rms << ",\n";
    out << "    \"peak\": " << snapshot.analyzer.peak << ",\n";
    out << "    \"loudness\": " << snapshot.analyzer.loudness << ",\n";
    out << "    \"gate\": " << snapshot.analyzer.gate << ",\n";
    out << "    \"confidence\": " << snapshot.analyzer.confidence << ",\n";
    out << "    \"active\": " << (snapshot.analyzer.active ? "true" : "false") << ",\n";
    out << "    \"sampleCounter\": " << snapshot.analyzer.sampleCounter << "\n";
    out << "  }\n";
    out << "}\n";
    return out.str();
}
}
