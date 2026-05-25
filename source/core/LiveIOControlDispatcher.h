#pragma once

#include "LiveIOBus.h"
#include "LiveIOMidiSendProof.h"

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace myworld
{
struct LiveIOControlFrame
{
    std::int64_t timestampMs = 0;
    LiveIOValueFrame values;
};

struct LiveIOOscFloatMessage
{
    std::string bindingId;
    std::string oscHost;
    int oscPort = 0;
    std::string oscAddress;
    double floatValue = 0.0;
};

struct LiveIOOscFloatSendResult
{
    bool sent = false;
    std::string error;
};

using LiveIOOscFloatSender = std::function<LiveIOOscFloatSendResult (const LiveIOOscFloatMessage& message)>;

struct LiveIOShaderUniformEvidence
{
    std::int64_t timestampMs = 0;
    std::string bindingId;
    std::string sourceId;
    std::string uniformName;
    double floatValue = 0.0;
};

struct LiveIOControlDispatchRequest
{
    std::vector<LiveIOControlFrame> frames;
    std::vector<LiveIOBinding> bindings;
    int minIntervalMs = 0;
    LiveIOMidiOutputInventory midiOutputInventory;
    std::string midiOutputIdentifier;
    LiveIOMidiOutputSender midiSender;
    LiveIOOscFloatSender oscSender;
    std::string oscHost;
    int oscPort = 0;
    bool midiEnabled = true;
    bool oscEnabled = true;
};

struct LiveIOControlDispatchFrameReport
{
    std::int64_t timestampMs = 0;
    bool dispatched = false;
    std::string status;
    int midiSentCount = 0;
    int oscSentCount = 0;
    int shaderSkippedCount = 0;
    std::vector<LiveIOShaderUniformEvidence> shaderUniforms;
    std::vector<std::string> errors;
};

struct LiveIOControlDispatchReport
{
    bool ok = false;
    std::string status;
    std::string message;
    int frameCount = 0;
    int dispatchedFrameCount = 0;
    int rateLimitedFrameCount = 0;
    int midiSentCount = 0;
    int oscSentCount = 0;
    int shaderSkippedCount = 0;
    std::vector<LiveIOShaderUniformEvidence> shaderUniforms;
    std::vector<LiveIOControlDispatchFrameReport> frames;
    std::vector<std::string> errors;
};

LiveIOControlDispatchReport executeLiveIOControlDispatch (const LiveIOControlDispatchRequest& request);
std::string makeLiveIOControlDispatchReportJson (const LiveIOControlDispatchReport& report);
}
