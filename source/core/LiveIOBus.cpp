#include "LiveIOBus.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace myworld
{
namespace
{
int clampInt (int value, int minimum, int maximum)
{
    return std::max (minimum, std::min (maximum, value));
}

double clampDouble (double value, double minimum, double maximum)
{
    if (! std::isfinite (value))
        return minimum;

    return std::max (minimum, std::min (maximum, value));
}

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

const LiveIOValue* findValue (const LiveIOValueFrame& frame, const std::string& id)
{
    const auto found = std::find_if (frame.values.begin(), frame.values.end(), [&] (const auto& value) {
        return value.id == id;
    });

    return found == frame.values.end() ? nullptr : &(*found);
}

double normalizedValueForBinding (double value, const LiveIOBinding& binding)
{
    const auto range = binding.inputMax - binding.inputMin;
    if (! std::isfinite (range) || std::abs (range) <= 0.000001)
        return 0.0;

    return clampDouble ((value - binding.inputMin) / range, 0.0, 1.0);
}

bool bindingTargetIsValid (const LiveIOBinding& binding, std::string& error)
{
    if (binding.id.empty())
    {
        error = "binding id is required";
        return false;
    }

    if (binding.sourceId.empty())
    {
        error = "binding source is required: " + binding.id;
        return false;
    }

    if (binding.targetKind == LiveIOTargetKind::oscFloat && binding.oscAddress.empty())
    {
        error = "osc address is required: " + binding.id;
        return false;
    }

    if (binding.targetKind == LiveIOTargetKind::shaderUniform && binding.uniformName.empty())
    {
        error = "shader uniform is required: " + binding.id;
        return false;
    }

    return true;
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

void appendEventsJson (std::ostringstream& out, const std::vector<LiveIOEvent>& events)
{
    out << "[\n";

    for (size_t index = 0; index < events.size(); ++index)
    {
        const auto& event = events[index];
        out << "    {\n";
        out << "      \"bindingId\": " << jsonQuoted (event.bindingId) << ",\n";
        out << "      \"sourceId\": " << jsonQuoted (event.sourceId) << ",\n";
        out << "      \"source\": " << jsonQuoted (event.source) << ",\n";
        out << "      \"targetKind\": " << jsonQuoted (liveIOTargetKindToString (event.targetKind)) << ",\n";
        out << "      \"inputValue\": " << event.inputValue << ",\n";
        out << "      \"normalizedValue\": " << event.normalizedValue << ",\n";
        out << "      \"floatValue\": " << event.floatValue << ",\n";
        out << "      \"channel\": " << event.midiChannel << ",\n";
        out << "      \"cc\": " << event.midiCc << ",\n";
        out << "      \"midiValue\": " << event.midiValue << ",\n";
        out << "      \"oscAddress\": " << jsonQuoted (event.oscAddress) << ",\n";
        out << "      \"uniformName\": " << jsonQuoted (event.uniformName) << "\n";
        out << "    }";

        if (index + 1 < events.size())
            out << ",";

        out << "\n";
    }

    out << "  ]";
}
}

LiveIOBinding makeLiveIOMidiCcBinding (const std::string& id,
                                       const std::string& sourceId,
                                       int channel,
                                       int cc)
{
    LiveIOBinding binding;
    binding.id = id;
    binding.sourceId = sourceId;
    binding.targetKind = LiveIOTargetKind::midiCc;
    binding.midiChannel = channel;
    binding.midiCc = cc;
    return binding;
}

LiveIOBinding makeLiveIOOscFloatBinding (const std::string& id,
                                         const std::string& sourceId,
                                         const std::string& address)
{
    LiveIOBinding binding;
    binding.id = id;
    binding.sourceId = sourceId;
    binding.targetKind = LiveIOTargetKind::oscFloat;
    binding.oscAddress = address;
    return binding;
}

LiveIOBinding makeLiveIOShaderUniformBinding (const std::string& id,
                                              const std::string& sourceId,
                                              const std::string& uniformName)
{
    LiveIOBinding binding;
    binding.id = id;
    binding.sourceId = sourceId;
    binding.targetKind = LiveIOTargetKind::shaderUniform;
    binding.uniformName = uniformName;
    return binding;
}

std::string liveIOTargetKindToString (LiveIOTargetKind kind)
{
    switch (kind)
    {
        case LiveIOTargetKind::midiCc:        return "midi.cc";
        case LiveIOTargetKind::oscFloat:      return "osc.float";
        case LiveIOTargetKind::shaderUniform: return "shader.uniform";
    }

    return "shader.uniform";
}

int liveIOMidiValueFromNormalized (double normalizedValue)
{
    return clampInt (static_cast<int> (std::round (clampDouble (normalizedValue, 0.0, 1.0) * 127.0)),
                     0,
                     127);
}

LiveIOBusReport evaluateLiveIOBus (const LiveIOValueFrame& frame,
                                   const std::vector<LiveIOBinding>& bindings)
{
    LiveIOBusReport report;

    for (const auto& binding : bindings)
    {
        std::string error;
        if (! bindingTargetIsValid (binding, error))
        {
            report.errors.push_back (error);
            continue;
        }

        const auto* value = findValue (frame, binding.sourceId);
        if (value == nullptr)
        {
            report.errors.push_back ("missing source: " + binding.sourceId);
            continue;
        }

        LiveIOEvent event;
        event.bindingId = binding.id;
        event.sourceId = binding.sourceId;
        event.source = value->source;
        event.targetKind = binding.targetKind;
        event.inputValue = value->value;
        event.normalizedValue = normalizedValueForBinding (value->value, binding);
        event.floatValue = event.normalizedValue;
        event.midiChannel = clampInt (binding.midiChannel, 1, 16);
        event.midiCc = clampInt (binding.midiCc, 0, 127);
        event.midiValue = liveIOMidiValueFromNormalized (event.normalizedValue);
        event.oscAddress = binding.oscAddress;
        event.uniformName = binding.uniformName;
        report.events.push_back (event);
    }

    report.ok = report.errors.empty();
    report.status = report.ok ? "mapped" : "blocked";
    report.message = report.ok ? "live_io_mapped" : report.errors.front();
    return report;
}

std::string makeLiveIOBusReportJson (const LiveIOBusReport& report)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"liveIOBusReport\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"status\": " << jsonQuoted (report.status) << ",\n";
    out << "  \"message\": " << jsonQuoted (report.message) << ",\n";
    out << "  \"events\": ";
    appendEventsJson (out, report.events);
    out << ",\n";
    out << "  \"errors\": ";
    appendErrorsJson (out, report.errors);
    out << "\n";
    out << "}\n";
    return out.str();
}
}
