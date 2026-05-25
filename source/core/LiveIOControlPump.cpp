#include "LiveIOControlPump.h"

#include <iomanip>
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

void appendTickReportsJson (std::ostringstream& out, const std::vector<LiveIOControlPumpTickReport>& ticks)
{
    out << "[\n";

    for (size_t index = 0; index < ticks.size(); ++index)
    {
        const auto& tick = ticks[index];
        out << "    {\n";
        out << "      \"timestampMs\": " << tick.timestampMs << ",\n";
        out << "      \"sampleCounter\": " << tick.sampleCounter << ",\n";
        out << "      \"loudness\": " << tick.loudness << ",\n";
        out << "      \"active\": " << (tick.active ? "true" : "false") << ",\n";
        out << "      \"framed\": " << (tick.framed ? "true" : "false") << ",\n";
        out << "      \"status\": " << jsonQuoted (tick.status) << "\n";
        out << "    }";

        if (index + 1 < ticks.size())
            out << ",";

        out << "\n";
    }

    out << "  ]";
}
}

LiveIOValueFrame makeLiveIOValueFrameFromAnalyzerSnapshot (const AudioAnalyzerSnapshot& snapshot)
{
    LiveIOValueFrame frame;
    frame.values = {
        { "out", snapshot.loudness, "AudioAnalyzerSnapshot.loudness" }
    };
    return frame;
}

LiveIOControlPumpReport executeLiveIOControlPump (const LiveIOControlPumpRequest& request)
{
    LiveIOControlPumpReport report;
    report.tickCount = static_cast<int> (request.ticks.size());

    if (request.ticks.empty())
    {
        report.status = "failed";
        report.message = "live io control pump ticks are required";
        report.errors.push_back (report.message);
        return report;
    }

    bool hasLastFrame = false;
    std::int64_t lastFrameMs = 0;
    const auto tickIntervalMs = request.tickIntervalMs < 0 ? 0 : request.tickIntervalMs;
    std::vector<LiveIOControlFrame> frames;

    for (const auto& tick : request.ticks)
    {
        LiveIOControlPumpTickReport tickReport;
        tickReport.timestampMs = tick.timestampMs;
        tickReport.sampleCounter = tick.snapshot.sampleCounter;
        tickReport.loudness = tick.snapshot.loudness;
        tickReport.active = tick.snapshot.active;

        report.lastSampleCounter = tick.snapshot.sampleCounter;

        if (! tick.snapshot.active)
        {
            tickReport.status = "inactive";
            ++report.inactiveTickCount;
            report.ticks.push_back (tickReport);
            continue;
        }

        if (hasLastFrame && tick.timestampMs - lastFrameMs < tickIntervalMs)
        {
            tickReport.status = "tick_rate_limited";
            ++report.tickRateLimitedCount;
            report.ticks.push_back (tickReport);
            continue;
        }

        tickReport.status = "framed";
        tickReport.framed = true;
        report.lastLoudness = tick.snapshot.loudness;
        frames.push_back ({ tick.timestampMs, makeLiveIOValueFrameFromAnalyzerSnapshot (tick.snapshot) });
        ++report.frameCount;
        hasLastFrame = true;
        lastFrameMs = tick.timestampMs;
        report.ticks.push_back (tickReport);
    }

    if (frames.empty())
    {
        report.status = "failed";
        report.message = "live io control pump produced no active frames";
        report.errors.push_back (report.message);
        return report;
    }

    LiveIOControlDispatchRequest dispatchRequest;
    dispatchRequest.frames = frames;
    dispatchRequest.bindings = request.bindings;
    dispatchRequest.minIntervalMs = request.dispatchMinIntervalMs;
    dispatchRequest.midiOutputInventory = request.midiOutputInventory;
    dispatchRequest.midiOutputIdentifier = request.midiOutputIdentifier;
    dispatchRequest.midiSender = request.midiSender;
    dispatchRequest.oscSender = request.oscSender;
    dispatchRequest.midiEnabled = request.midiEnabled;
    dispatchRequest.oscEnabled = request.oscEnabled;
    report.dispatch = executeLiveIOControlDispatch (dispatchRequest);

    if (! report.dispatch.ok)
    {
        report.ok = false;
        report.status = "failed";
        report.message = report.dispatch.message;
        report.errors = report.dispatch.errors;
        return report;
    }

    report.ok = true;
    report.status = "pumped";
    report.message = "live_io_control_pump_complete";
    return report;
}

std::string makeLiveIOControlPumpReportJson (const LiveIOControlPumpReport& report)
{
    const auto dispatchJson = makeLiveIOControlDispatchReportJson (report.dispatch);
    const auto dispatchJsonEnd = dispatchJson.find_last_not_of (" \t\r\n");
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"liveIOControlPumpProof\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"status\": " << jsonQuoted (report.status) << ",\n";
    out << "  \"message\": " << jsonQuoted (report.message) << ",\n";
    out << "  \"tickCount\": " << report.tickCount << ",\n";
    out << "  \"frameCount\": " << report.frameCount << ",\n";
    out << "  \"inactiveTickCount\": " << report.inactiveTickCount << ",\n";
    out << "  \"tickRateLimitedCount\": " << report.tickRateLimitedCount << ",\n";
    out << "  \"lastLoudness\": " << report.lastLoudness << ",\n";
    out << "  \"lastSampleCounter\": " << report.lastSampleCounter << ",\n";
    out << "  \"ticks\": ";
    appendTickReportsJson (out, report.ticks);
    out << ",\n";
    out << "  \"dispatch\": " << dispatchJson.substr (0, dispatchJsonEnd + 1) << ",\n";
    out << "  \"errors\": ";
    appendErrorsJson (out, report.errors);
    out << "\n";
    out << "}\n";
    return out.str();
}
}
