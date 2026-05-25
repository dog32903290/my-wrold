#include "GraphIOMapping.h"

#include "JsonWriter.h"

#include <iomanip>
#include <sstream>

namespace myworld
{
namespace
{
void addDiagnostic (std::vector<std::string>& diagnostics, const std::string& diagnostic)
{
    diagnostics.push_back (diagnostic);
}

void appendDiagnosticsJson (std::ostringstream& out, const std::vector<std::string>& diagnostics)
{
    appendJsonStringArray (out, diagnostics);
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
        out << "      \"uniformName\": " << jsonQuoted (event.uniformName) << "\n";
        out << "    }";

        if (index + 1 < events.size())
            out << ",";

        out << "\n";
    }

    out << "  ]";
}
}

GraphIOMappingValidation validateGraphIOMapping (const GraphIOMapping& mapping)
{
    GraphIOMappingValidation validation;

    if (mapping.id.empty())
        addDiagnostic (validation.diagnostics, "mapping id is required");

    if (mapping.source.endpoint.empty())
        addDiagnostic (validation.diagnostics, "source endpoint is required: " + mapping.id);

    if (mapping.source.dataType != "signal.float")
        addDiagnostic (validation.diagnostics, "source data type must be signal.float: " + mapping.id);

    if (mapping.source.streamKind != "continuous")
        addDiagnostic (validation.diagnostics, "source stream kind must be continuous: " + mapping.id);

    if (mapping.target.kind != "shader.uniform")
        addDiagnostic (validation.diagnostics, "target kind must be shader.uniform: " + mapping.id);

    if (mapping.target.id.empty())
        addDiagnostic (validation.diagnostics, "target id is required: " + mapping.id);

    if (mapping.target.uniformName.empty())
        addDiagnostic (validation.diagnostics, "shader uniform is required: " + mapping.id);

    if (mapping.target.dataType != "signal.float")
        addDiagnostic (validation.diagnostics, "target data type must be signal.float: " + mapping.id);

    validation.ok = validation.diagnostics.empty();
    validation.message = validation.ok ? "graph_io_mapping_valid" : validation.diagnostics.front();
    return validation;
}

LiveIOBinding makeLiveIOBindingFromGraphIOMapping (const GraphIOMapping& mapping)
{
    auto binding = makeLiveIOShaderUniformBinding (mapping.id, mapping.source.endpoint, mapping.target.uniformName);
    binding.inputMin = mapping.inputMin;
    binding.inputMax = mapping.inputMax;
    return binding;
}

GraphIOMappingReport evaluateGraphIOMapping (const GraphIOMapping& mapping,
                                             const LiveIOValueFrame& frame)
{
    GraphIOMappingReport report;
    report.mapping = mapping;

    const auto validation = validateGraphIOMapping (mapping);
    if (! validation.ok)
    {
        report.ok = false;
        report.status = "blocked";
        report.message = validation.message;
        report.diagnostics = validation.diagnostics;
        return report;
    }

    const auto busReport = evaluateLiveIOBus (frame, { makeLiveIOBindingFromGraphIOMapping (mapping) });
    report.ok = busReport.ok;
    report.status = busReport.status;
    report.message = busReport.message;
    report.events = busReport.events;
    report.diagnostics = busReport.errors;
    return report;
}

std::string makeGraphIOMappingReportJson (const GraphIOMappingReport& report)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision (6);
    out << "{\n";
    out << "  \"kind\": \"graphIOMappingReport\",\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"status\": " << jsonQuoted (report.status) << ",\n";
    out << "  \"message\": " << jsonQuoted (report.message) << ",\n";
    out << "  \"mappingId\": " << jsonQuoted (report.mapping.id) << ",\n";
    out << "  \"sourceEndpoint\": " << jsonQuoted (report.mapping.source.endpoint) << ",\n";
    out << "  \"sourceDataType\": " << jsonQuoted (report.mapping.source.dataType) << ",\n";
    out << "  \"sourceStreamKind\": " << jsonQuoted (report.mapping.source.streamKind) << ",\n";
    out << "  \"targetId\": " << jsonQuoted (report.mapping.target.id) << ",\n";
    out << "  \"targetKind\": " << jsonQuoted (report.mapping.target.kind) << ",\n";
    out << "  \"uniformName\": " << jsonQuoted (report.mapping.target.uniformName) << ",\n";
    out << "  \"events\": ";
    appendEventsJson (out, report.events);
    out << ",\n";
    out << "  \"diagnostics\": ";
    appendDiagnosticsJson (out, report.diagnostics);
    out << "\n";
    out << "}\n";
    return out.str();
}
}
