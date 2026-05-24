#include "RuntimeRegistryInternals.h"

#include "AudioAnalyzerState.h"

#include <algorithm>
#include <cmath>

namespace myworld
{
namespace runtime_registry_internal
{
namespace
{
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
}

const RuntimeOutputValue* findOutputValue (const std::vector<RuntimeOutputValue>& values, const std::string& id)
{
    const auto found = std::find_if (values.begin(), values.end(), [&id] (const auto& value) {
        return value.id == id;
    });

    return found == values.end() ? nullptr : &*found;
}

const RuntimeRegistryChild* findRuntimeChild (const RuntimeRegistryEntry& entry, const std::string& childId)
{
    const auto found = std::find_if (entry.children.begin(), entry.children.end(), [&childId] (const auto& child) {
        return child.id == childId;
    });

    return found == entry.children.end() ? nullptr : &(*found);
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
}

namespace
{
using namespace runtime_registry_internal;

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

LoudnessRuntimeBridgeSnapshot makeLoudnessRuntimeFallbackSnapshot (const AudioAnalyzerSnapshot& fallbackSnapshot)
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
    using namespace runtime_registry_internal;

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
}
