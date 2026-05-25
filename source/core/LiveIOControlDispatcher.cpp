#include "LiveIOControlDispatcher.h"

#include <sstream>

namespace myworld
{
namespace
{
std::string jsonQuoted (const std::string& text)
{
    std::ostringstream out;
    out << '"';

    for (const auto character : text)
    {
        switch (character)
        {
            case '\\': out << "\\\\"; break;
            case '"':  out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:   out << character; break;
        }
    }

    out << '"';
    return out.str();
}

void appendErrorsJson (std::ostringstream& out, const std::vector<std::string>& errors)
{
    out << "[";

    for (size_t index = 0; index < errors.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << jsonQuoted (errors[index]);
    }

    out << "]";
}

std::string selectedMidiIdentifier (const LiveIOControlDispatchRequest& request)
{
    if (! request.midiOutputIdentifier.empty())
        return request.midiOutputIdentifier;

    if (! request.midiOutputInventory.devices.empty())
        return request.midiOutputInventory.devices.front().identifier;

    return "__no_live_io_midi_outputs__";
}

LiveIOMidiCcMessage midiMessageFromEvent (const LiveIOEvent& event)
{
    LiveIOMidiCcMessage message;
    message.bindingId = event.bindingId;
    message.channel = event.midiChannel;
    message.cc = event.midiCc;
    message.value = event.midiValue;
    return message;
}

LiveIOOscFloatMessage oscMessageFromEvent (const LiveIOEvent& event,
                                           const LiveIOControlDispatchRequest& request)
{
    LiveIOOscFloatMessage message;
    message.bindingId = event.bindingId;
    message.oscHost = request.oscHost;
    message.oscPort = request.oscPort;
    message.oscAddress = event.oscAddress;
    message.floatValue = event.floatValue;
    return message;
}

void recordFrameError (LiveIOControlDispatchReport& report,
                       LiveIOControlDispatchFrameReport& frameReport,
                       const std::string& error)
{
    frameReport.errors.push_back (error);
    report.errors.push_back (error);
}

void appendFrameReportsJson (std::ostringstream& out,
                             const std::vector<LiveIOControlDispatchFrameReport>& frames)
{
    out << "[\n";

    for (size_t index = 0; index < frames.size(); ++index)
    {
        const auto& frame = frames[index];
        out << "    {\n";
        out << "      \"timestampMs\": " << frame.timestampMs << ",\n";
        out << "      \"dispatched\": " << (frame.dispatched ? "true" : "false") << ",\n";
        out << "      \"status\": " << jsonQuoted (frame.status) << ",\n";
        out << "      \"midiSentCount\": " << frame.midiSentCount << ",\n";
        out << "      \"oscSentCount\": " << frame.oscSentCount << ",\n";
        out << "      \"shaderSkippedCount\": " << frame.shaderSkippedCount << ",\n";
        out << "      \"errors\": ";
        appendErrorsJson (out, frame.errors);
        out << "\n";
        out << "    }";

        if (index + 1 < frames.size())
            out << ",";

        out << "\n";
    }

    out << "  ]";
}
}

LiveIOControlDispatchReport executeLiveIOControlDispatch (const LiveIOControlDispatchRequest& request)
{
    LiveIOControlDispatchReport report;
    report.frameCount = static_cast<int> (request.frames.size());

    if (request.frames.empty())
    {
        report.status = "failed";
        report.message = "live io control frames are required";
        report.errors.push_back (report.message);
        return report;
    }

    bool hasLastDispatch = false;
    std::int64_t lastDispatchMs = 0;
    const auto intervalMs = request.minIntervalMs < 0 ? 0 : request.minIntervalMs;

    for (const auto& frame : request.frames)
    {
        LiveIOControlDispatchFrameReport frameReport;
        frameReport.timestampMs = frame.timestampMs;

        if (hasLastDispatch && frame.timestampMs - lastDispatchMs < intervalMs)
        {
            frameReport.status = "rate_limited";
            ++report.rateLimitedFrameCount;
            report.frames.push_back (frameReport);
            continue;
        }

        const auto busReport = evaluateLiveIOBus (frame.values, request.bindings);
        if (! busReport.ok)
        {
            frameReport.status = "failed";
            recordFrameError (report, frameReport, busReport.message);
            report.frames.push_back (frameReport);
            continue;
        }

        frameReport.dispatched = true;
        frameReport.status = "dispatched";
        ++report.dispatchedFrameCount;
        hasLastDispatch = true;
        lastDispatchMs = frame.timestampMs;

        for (const auto& event : busReport.events)
        {
            if (event.targetKind == LiveIOTargetKind::shaderUniform)
            {
                ++frameReport.shaderSkippedCount;
                ++report.shaderSkippedCount;
                continue;
            }

            if (event.targetKind == LiveIOTargetKind::midiCc)
            {
                if (! request.midiEnabled)
                    continue;

                const auto midiReport = executeLiveIOMidiOutputSendProof (
                    request.midiOutputInventory,
                    makeLiveIOMidiOutputSendRequestByIdentifier (
                        selectedMidiIdentifier (request),
                        midiMessageFromEvent (event)),
                    request.midiSender);

                if (! midiReport.ok)
                {
                    frameReport.status = "failed";
                    recordFrameError (report, frameReport, midiReport.message);
                    continue;
                }

                if (midiReport.sent)
                {
                    ++frameReport.midiSentCount;
                    ++report.midiSentCount;
                }

                continue;
            }

            if (event.targetKind == LiveIOTargetKind::oscFloat)
            {
                if (! request.oscEnabled)
                    continue;

                if (! request.oscSender)
                {
                    frameReport.status = "failed";
                    recordFrameError (report, frameReport, "osc sender is unavailable: " + event.bindingId);
                    continue;
                }

                const auto oscResult = request.oscSender (oscMessageFromEvent (event, request));
                if (! oscResult.sent)
                {
                    frameReport.status = "failed";
                    recordFrameError (
                        report,
                        frameReport,
                        oscResult.error.empty() ? "osc send failed: " + event.bindingId : oscResult.error);
                    continue;
                }

                ++frameReport.oscSentCount;
                ++report.oscSentCount;
            }
        }

        report.frames.push_back (frameReport);
    }

    report.ok = report.errors.empty();
    report.status = report.ok ? "dispatched" : "failed";
    report.message = report.ok ? "live_io_control_dispatch_complete" : report.errors.front();
    return report;
}

std::string makeLiveIOControlDispatchReportJson (const LiveIOControlDispatchReport& report)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"kind\": \"liveIOControlDispatchProof\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"status\": " << jsonQuoted (report.status) << ",\n";
    out << "  \"message\": " << jsonQuoted (report.message) << ",\n";
    out << "  \"frameCount\": " << report.frameCount << ",\n";
    out << "  \"dispatchedFrameCount\": " << report.dispatchedFrameCount << ",\n";
    out << "  \"rateLimitedFrameCount\": " << report.rateLimitedFrameCount << ",\n";
    out << "  \"midiSentCount\": " << report.midiSentCount << ",\n";
    out << "  \"oscSentCount\": " << report.oscSentCount << ",\n";
    out << "  \"shaderSkippedCount\": " << report.shaderSkippedCount << ",\n";
    out << "  \"frames\": ";
    appendFrameReportsJson (out, report.frames);
    out << ",\n";
    out << "  \"errors\": ";
    appendErrorsJson (out, report.errors);
    out << "\n";
    out << "}\n";
    return out.str();
}
}
