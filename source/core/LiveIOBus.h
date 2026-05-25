#pragma once

#include <string>
#include <vector>

namespace myworld
{
enum class LiveIOTargetKind
{
    midiCc,
    oscFloat,
    shaderUniform
};

struct LiveIOValue
{
    std::string id;
    double value = 0.0;
    std::string source;
};

struct LiveIOValueFrame
{
    std::vector<LiveIOValue> values;
};

struct LiveIOBinding
{
    std::string id;
    std::string sourceId;
    LiveIOTargetKind targetKind = LiveIOTargetKind::shaderUniform;
    int midiChannel = 1;
    int midiCc = 0;
    std::string oscAddress;
    std::string uniformName;
    double inputMin = 0.0;
    double inputMax = 1.0;
};

struct LiveIOEvent
{
    std::string bindingId;
    std::string sourceId;
    std::string source;
    LiveIOTargetKind targetKind = LiveIOTargetKind::shaderUniform;
    double inputValue = 0.0;
    double normalizedValue = 0.0;
    double floatValue = 0.0;
    int midiChannel = 1;
    int midiCc = 0;
    int midiValue = 0;
    std::string oscAddress;
    std::string uniformName;
};

struct LiveIOBusReport
{
    bool ok = false;
    std::string status;
    std::string message;
    std::vector<LiveIOEvent> events;
    std::vector<std::string> errors;
};

LiveIOBinding makeLiveIOMidiCcBinding (const std::string& id,
                                       const std::string& sourceId,
                                       int channel,
                                       int cc);
LiveIOBinding makeLiveIOOscFloatBinding (const std::string& id,
                                         const std::string& sourceId,
                                         const std::string& address);
LiveIOBinding makeLiveIOShaderUniformBinding (const std::string& id,
                                              const std::string& sourceId,
                                              const std::string& uniformName);

std::string liveIOTargetKindToString (LiveIOTargetKind kind);
int liveIOMidiValueFromNormalized (double normalizedValue);
LiveIOBusReport evaluateLiveIOBus (const LiveIOValueFrame& frame,
                                   const std::vector<LiveIOBinding>& bindings);
std::string makeLiveIOBusReportJson (const LiveIOBusReport& report);
}
