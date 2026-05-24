#include "RuntimeRegistryInternals.h"

#include "AnalyzerAggregatePressure.h"
#include "AnalyzerAttackDetector.h"
#include "AnalyzerDensityDetector.h"
#include "AnalyzerResidueDetector.h"
#include "AnalyzerSilenceDetector.h"
#include "AnalyzerSustainDetector.h"
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

void runSyntheticAttackOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
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
        AttackDetector detector { {} };
        const auto output = detector.processFrame ({ true,
                                                     rmsInput.value,
                                                     true,
                                                     peakInput.value,
                                                     true,
                                                     sampleCountInput.value,
                                                     0.0 });
        childStatus.status = output.detectorOk ? "computed" : "blocked";
        childStatus.reason = output.detectorOk ? "computed attack detector state from raw energy facts"
                                               : output.diagnostic;
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "onset_event",
                      output.onsetEvent ? 1.0 : 0.0);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "attack_value",
                      output.attackValue);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "attack_envelope",
                      output.attackEnvelope);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "confidence",
                      output.confidence);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for raw energy facts";
    }
}

void runSyntheticAttackOutOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto onsetInput = makeInputValue ("onset_event",
                                            context.entry,
                                            context.child.id,
                                            "onset_event",
                                            context.valueBus);
    const auto attackValueInput = makeInputValue ("attack_value",
                                                  context.entry,
                                                  context.child.id,
                                                  "attack_value",
                                                  context.valueBus);
    const auto attackEnvelopeInput = makeInputValue ("attack_envelope",
                                                     context.entry,
                                                     context.child.id,
                                                     "attack_envelope",
                                                     context.valueBus);
    const auto confidenceInput = makeInputValue ("confidence",
                                                 context.entry,
                                                 context.child.id,
                                                 "confidence",
                                                 context.valueBus);
    childStatus.inputs = {
        onsetInput,
        attackValueInput,
        attackEnvelopeInput,
        confidenceInput
    };

    if (hasValue (context.valueBus, onsetInput.source)
        && hasValue (context.valueBus, attackValueInput.source)
        && hasValue (context.valueBus, attackEnvelopeInput.source)
        && hasValue (context.valueBus, confidenceInput.source))
    {
        childStatus.status = "computed";
        childStatus.reason = "published attack detector public outputs";
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "onset_event", onsetInput.value);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "attack_value", attackValueInput.value);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "attack_envelope",
                      attackEnvelopeInput.value);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "confidence", confidenceInput.value);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for analyzer.attack output";
    }
}

void runSyntheticDensityOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto onsetInput = makeInputValue ("onset_event",
                                            context.entry,
                                            context.child.id,
                                            "onset_event",
                                            context.valueBus);
    const auto attackValueInput = makeInputValue ("attack_value",
                                                  context.entry,
                                                  context.child.id,
                                                  "attack_value",
                                                  context.valueBus);
    childStatus.inputs = {
        onsetInput,
        attackValueInput
    };

    if (hasValue (context.valueBus, onsetInput.source))
    {
        DensityDetector detector { {} };
        const auto output = detector.processFrame ({ true,
                                                     onsetInput.value != 0.0,
                                                     hasValue (context.valueBus, attackValueInput.source),
                                                     attackValueInput.value,
                                                     false,
                                                     0.0,
                                                     0.0 });
        childStatus.status = output.detectorOk ? "computed" : "blocked";
        childStatus.reason = output.detectorOk ? "computed density from onset events"
                                               : output.diagnostic;
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "density_value",
                      output.densityValue);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "event_count",
                      static_cast<double> (output.eventCount));
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "density_envelope",
                      output.densityEnvelope);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "confidence", output.confidence);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for onset_event";
    }
}

void runSyntheticDensityOutOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto densityValueInput = makeInputValue ("density_value",
                                                   context.entry,
                                                   context.child.id,
                                                   "density_value",
                                                   context.valueBus);
    const auto eventCountInput = makeInputValue ("event_count",
                                                 context.entry,
                                                 context.child.id,
                                                 "event_count",
                                                 context.valueBus);
    const auto densityEnvelopeInput = makeInputValue ("density_envelope",
                                                      context.entry,
                                                      context.child.id,
                                                      "density_envelope",
                                                      context.valueBus);
    const auto confidenceInput = makeInputValue ("confidence",
                                                 context.entry,
                                                 context.child.id,
                                                 "confidence",
                                                 context.valueBus);
    childStatus.inputs = {
        densityValueInput,
        eventCountInput,
        densityEnvelopeInput,
        confidenceInput
    };

    if (hasValue (context.valueBus, densityValueInput.source)
        && hasValue (context.valueBus, eventCountInput.source)
        && hasValue (context.valueBus, densityEnvelopeInput.source)
        && hasValue (context.valueBus, confidenceInput.source))
    {
        childStatus.status = "computed";
        childStatus.reason = "published density detector public outputs";
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "density_value",
                      densityValueInput.value);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "event_count", eventCountInput.value);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "density_envelope",
                      densityEnvelopeInput.value);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "confidence", confidenceInput.value);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for analyzer.density output";
    }
}

void runSyntheticSilenceOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto rmsInput = makeInputValue ("rms", context.entry, context.child.id, "rms", context.valueBus);
    const auto peakInput = makeInputValue ("peak", context.entry, context.child.id, "peak", context.valueBus);
    childStatus.inputs = {
        rmsInput,
        peakInput
    };

    if (hasValue (context.valueBus, rmsInput.source))
    {
        SilenceDetector detector { {} };
        const auto output = detector.processFrame ({ true,
                                                     rmsInput.value,
                                                     hasValue (context.valueBus, peakInput.source),
                                                     peakInput.value,
                                                     0.0 });
        childStatus.status = output.detectorOk ? "computed" : "blocked";
        childStatus.reason = output.detectorOk ? "computed sustained quiet state from raw RMS"
                                               : output.diagnostic;
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "silence_state",
                      output.silenceState ? 1.0 : 0.0);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "silence_timer_ms",
                      output.silenceTimerMs);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "confidence", output.confidence);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for raw RMS";
    }
}

void runSyntheticSilenceOutOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto silenceStateInput = makeInputValue ("silence_state",
                                                   context.entry,
                                                   context.child.id,
                                                   "silence_state",
                                                   context.valueBus);
    const auto silenceTimerInput = makeInputValue ("silence_timer_ms",
                                                   context.entry,
                                                   context.child.id,
                                                   "silence_timer_ms",
                                                   context.valueBus);
    const auto confidenceInput = makeInputValue ("confidence",
                                                 context.entry,
                                                 context.child.id,
                                                 "confidence",
                                                 context.valueBus);
    childStatus.inputs = {
        silenceStateInput,
        silenceTimerInput,
        confidenceInput
    };

    if (hasValue (context.valueBus, silenceStateInput.source)
        && hasValue (context.valueBus, silenceTimerInput.source)
        && hasValue (context.valueBus, confidenceInput.source))
    {
        childStatus.status = "computed";
        childStatus.reason = "published silence detector public outputs";
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "silence_state",
                      silenceStateInput.value);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "silence_timer_ms",
                      silenceTimerInput.value);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "confidence", confidenceInput.value);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for analyzer.silence output";
    }
}

void runSyntheticSustainOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto rmsInput = makeInputValue ("rms", context.entry, context.child.id, "rms", context.valueBus);
    const auto peakInput = makeInputValue ("peak", context.entry, context.child.id, "peak", context.valueBus);
    childStatus.inputs = {
        rmsInput,
        peakInput
    };

    if (hasValue (context.valueBus, rmsInput.source))
    {
        SustainDetector detector { {} };
        const auto output = detector.processFrame ({ true,
                                                     rmsInput.value,
                                                     hasValue (context.valueBus, peakInput.source),
                                                     peakInput.value,
                                                     0.0 });
        childStatus.status = output.detectorOk ? "computed" : "blocked";
        childStatus.reason = output.detectorOk ? "computed held active energy from raw RMS"
                                               : output.diagnostic;
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "sustain_state",
                      output.sustainState ? 1.0 : 0.0);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "sustain_timer_ms",
                      output.sustainTimerMs);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "sustain_envelope",
                      output.sustainEnvelope);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "confidence", output.confidence);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for raw RMS";
    }
}

void runSyntheticSustainOutOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto sustainStateInput = makeInputValue ("sustain_state",
                                                   context.entry,
                                                   context.child.id,
                                                   "sustain_state",
                                                   context.valueBus);
    const auto sustainTimerInput = makeInputValue ("sustain_timer_ms",
                                                   context.entry,
                                                   context.child.id,
                                                   "sustain_timer_ms",
                                                   context.valueBus);
    const auto sustainEnvelopeInput = makeInputValue ("sustain_envelope",
                                                      context.entry,
                                                      context.child.id,
                                                      "sustain_envelope",
                                                      context.valueBus);
    const auto confidenceInput = makeInputValue ("confidence",
                                                 context.entry,
                                                 context.child.id,
                                                 "confidence",
                                                 context.valueBus);
    childStatus.inputs = {
        sustainStateInput,
        sustainTimerInput,
        sustainEnvelopeInput,
        confidenceInput
    };

    if (hasValue (context.valueBus, sustainStateInput.source)
        && hasValue (context.valueBus, sustainTimerInput.source)
        && hasValue (context.valueBus, sustainEnvelopeInput.source)
        && hasValue (context.valueBus, confidenceInput.source))
    {
        childStatus.status = "computed";
        childStatus.reason = "published sustain detector public outputs";
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "sustain_state",
                      sustainStateInput.value);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "sustain_timer_ms",
                      sustainTimerInput.value);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "sustain_envelope",
                      sustainEnvelopeInput.value);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "confidence", confidenceInput.value);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for analyzer.sustain output";
    }
}

void runSyntheticResidueOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto rmsInput = makeInputValue ("rms", context.entry, context.child.id, "rms", context.valueBus);
    const auto sustainEnvelopeInput = makeInputValue ("sustain_envelope",
                                                      context.entry,
                                                      context.child.id,
                                                      "sustain_envelope",
                                                      context.valueBus);
    const auto silenceStateInput = makeInputValue ("silence_state",
                                                   context.entry,
                                                   context.child.id,
                                                   "silence_state",
                                                   context.valueBus);
    childStatus.inputs = {
        rmsInput,
        sustainEnvelopeInput,
        silenceStateInput
    };

    if (hasValue (context.valueBus, rmsInput.source)
        && hasValue (context.valueBus, sustainEnvelopeInput.source))
    {
        ResidueDetector detector { {} };
        const auto output = detector.processFrame ({ true,
                                                     rmsInput.value,
                                                     true,
                                                     sustainEnvelopeInput.value,
                                                     hasValue (context.valueBus, silenceStateInput.source),
                                                     silenceStateInput.value != 0.0,
                                                     0.0 });
        childStatus.status = output.detectorOk ? "computed" : "blocked";
        childStatus.reason = output.detectorOk ? "computed residue tail from raw RMS and sustain state"
                                               : output.diagnostic;
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "residue_state",
                      output.residueState ? 1.0 : 0.0);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "residue_envelope",
                      output.residueEnvelope);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "residue_timer_ms",
                      output.residueTimerMs);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "confidence", output.confidence);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for raw RMS and sustain envelope";
    }
}

void runSyntheticResidueOutOp (SyntheticRuntimeOpContext& context, RuntimeChildExecutionStatus& childStatus)
{
    const auto residueStateInput = makeInputValue ("residue_state",
                                                   context.entry,
                                                   context.child.id,
                                                   "residue_state",
                                                   context.valueBus);
    const auto residueEnvelopeInput = makeInputValue ("residue_envelope",
                                                      context.entry,
                                                      context.child.id,
                                                      "residue_envelope",
                                                      context.valueBus);
    const auto residueTimerInput = makeInputValue ("residue_timer_ms",
                                                   context.entry,
                                                   context.child.id,
                                                   "residue_timer_ms",
                                                   context.valueBus);
    const auto confidenceInput = makeInputValue ("confidence",
                                                 context.entry,
                                                 context.child.id,
                                                 "confidence",
                                                 context.valueBus);
    childStatus.inputs = {
        residueStateInput,
        residueEnvelopeInput,
        residueTimerInput,
        confidenceInput
    };

    if (hasValue (context.valueBus, residueStateInput.source)
        && hasValue (context.valueBus, residueEnvelopeInput.source)
        && hasValue (context.valueBus, residueTimerInput.source)
        && hasValue (context.valueBus, confidenceInput.source))
    {
        childStatus.status = "computed";
        childStatus.reason = "published residue detector public outputs";
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "residue_state",
                      residueStateInput.value);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "residue_envelope",
                      residueEnvelopeInput.value);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "residue_timer_ms",
                      residueTimerInput.value);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "confidence", confidenceInput.value);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for analyzer.residue output";
    }
}

void runSyntheticAggregatePressureOp (SyntheticRuntimeOpContext& context,
                                      RuntimeChildExecutionStatus& childStatus)
{
    const auto rmsInput = makeInputValue ("rms", context.entry, context.child.id, "rms", context.valueBus);
    const auto attackInput = makeInputValue ("attack_value",
                                             context.entry,
                                             context.child.id,
                                             "attack_value",
                                             context.valueBus);
    const auto densityInput = makeInputValue ("density_value",
                                              context.entry,
                                              context.child.id,
                                              "density_value",
                                              context.valueBus);
    const auto sustainInput = makeInputValue ("sustain_envelope",
                                              context.entry,
                                              context.child.id,
                                              "sustain_envelope",
                                              context.valueBus);
    const auto residueInput = makeInputValue ("residue_envelope",
                                              context.entry,
                                              context.child.id,
                                              "residue_envelope",
                                              context.valueBus);
    const auto silenceInput = makeInputValue ("silence_state",
                                              context.entry,
                                              context.child.id,
                                              "silence_state",
                                              context.valueBus);
    childStatus.inputs = {
        rmsInput,
        attackInput,
        densityInput,
        sustainInput,
        residueInput,
        silenceInput
    };

    if (hasValue (context.valueBus, rmsInput.source)
        && hasValue (context.valueBus, attackInput.source)
        && hasValue (context.valueBus, densityInput.source)
        && hasValue (context.valueBus, sustainInput.source)
        && hasValue (context.valueBus, residueInput.source)
        && hasValue (context.valueBus, silenceInput.source))
    {
        AggregatePressure aggregate { {} };
        const auto output = aggregate.processFrame ({ true,
                                                      rmsInput.value,
                                                      true,
                                                      attackInput.value,
                                                      true,
                                                      densityInput.value,
                                                      true,
                                                      sustainInput.value,
                                                      true,
                                                      residueInput.value,
                                                      true,
                                                      silenceInput.value != 0.0 });
        childStatus.status = output.aggregateOk ? "computed" : "blocked";
        childStatus.reason = output.aggregateOk ? "computed aggregate pressure from weighted detector states"
                                                : output.diagnostic;
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "pressure_value",
                      output.pressureValue);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "energy_component",
                      output.energyComponent);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "attack_component",
                      output.attackComponent);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "density_component",
                      output.densityComponent);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "sustain_component",
                      output.sustainComponent);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "residue_component",
                      output.residueComponent);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "confidence", output.confidence);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for aggregate pressure inputs";
    }
}

void runSyntheticAggregatePressureOutOp (SyntheticRuntimeOpContext& context,
                                         RuntimeChildExecutionStatus& childStatus)
{
    const auto pressureInput = makeInputValue ("pressure_value",
                                               context.entry,
                                               context.child.id,
                                               "pressure_value",
                                               context.valueBus);
    const auto energyInput = makeInputValue ("energy_component",
                                             context.entry,
                                             context.child.id,
                                             "energy_component",
                                             context.valueBus);
    const auto attackInput = makeInputValue ("attack_component",
                                             context.entry,
                                             context.child.id,
                                             "attack_component",
                                             context.valueBus);
    const auto densityInput = makeInputValue ("density_component",
                                              context.entry,
                                              context.child.id,
                                              "density_component",
                                              context.valueBus);
    const auto sustainInput = makeInputValue ("sustain_component",
                                              context.entry,
                                              context.child.id,
                                              "sustain_component",
                                              context.valueBus);
    const auto residueInput = makeInputValue ("residue_component",
                                              context.entry,
                                              context.child.id,
                                              "residue_component",
                                              context.valueBus);
    const auto confidenceInput = makeInputValue ("confidence",
                                                 context.entry,
                                                 context.child.id,
                                                 "confidence",
                                                 context.valueBus);
    childStatus.inputs = {
        pressureInput,
        energyInput,
        attackInput,
        densityInput,
        sustainInput,
        residueInput,
        confidenceInput
    };

    if (hasValue (context.valueBus, pressureInput.source)
        && hasValue (context.valueBus, energyInput.source)
        && hasValue (context.valueBus, attackInput.source)
        && hasValue (context.valueBus, densityInput.source)
        && hasValue (context.valueBus, sustainInput.source)
        && hasValue (context.valueBus, residueInput.source)
        && hasValue (context.valueBus, confidenceInput.source))
    {
        childStatus.status = "computed";
        childStatus.reason = "published aggregate pressure public outputs";
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "pressure_value",
                      pressureInput.value);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "energy_component",
                      energyInput.value);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "attack_component",
                      attackInput.value);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "density_component",
                      densityInput.value);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "sustain_component",
                      sustainInput.value);
        publishValue (context.valueBus,
                      childStatus.outputs,
                      context.child.id,
                      "residue_component",
                      residueInput.value);
        publishValue (context.valueBus, childStatus.outputs, context.child.id, "confidence", confidenceInput.value);
    }
    else
    {
        childStatus.status = "blocked";
        childStatus.reason = "waiting for analyzer.aggregate_pressure output";
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
        { "analyzer.attack", "synthetic.analyzer.attack", runSyntheticAttackOp },
        { "analyzer.attack_out", "synthetic.analyzer.attack_out", runSyntheticAttackOutOp },
        { "analyzer.density", "synthetic.analyzer.density", runSyntheticDensityOp },
        { "analyzer.density_out", "synthetic.analyzer.density_out", runSyntheticDensityOutOp },
        { "analyzer.silence", "synthetic.analyzer.silence", runSyntheticSilenceOp },
        { "analyzer.silence_out", "synthetic.analyzer.silence_out", runSyntheticSilenceOutOp },
        { "analyzer.sustain", "synthetic.analyzer.sustain", runSyntheticSustainOp },
        { "analyzer.sustain_out", "synthetic.analyzer.sustain_out", runSyntheticSustainOutOp },
        { "analyzer.residue", "synthetic.analyzer.residue", runSyntheticResidueOp },
        { "analyzer.residue_out", "synthetic.analyzer.residue_out", runSyntheticResidueOutOp },
        { "analyzer.aggregate_pressure", "synthetic.analyzer.aggregate_pressure", runSyntheticAggregatePressureOp },
        { "analyzer.aggregate_pressure_out",
          "synthetic.analyzer.aggregate_pressure_out",
          runSyntheticAggregatePressureOutOp },
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
