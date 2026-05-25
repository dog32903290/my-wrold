#include "GraphIOMapping.h"
#include "GraphIOMappingStorage.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (condition)
        return;

    std::cerr << "FAIL: " << message << '\n';
    std::exit (1);
}

void expectEqual (const std::string& actual, const std::string& expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + expected + " got " + actual);
}

void expectNear (double actual, double expected, double tolerance, const std::string& message)
{
    expect (std::abs (actual - expected) <= tolerance,
            message + " expected near " + std::to_string (expected) + " got " + std::to_string (actual));
}

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}
}

int main()
{
    const auto loaded = myworld::loadGraphIOMappingsFromFile (
        "fixtures/graphs/g1_loudness_to_shader_uniform.graph.json");

    expect (loaded.ok, loaded.error);
    expect (loaded.mappings.size() == 1, "loads one graph io mapping");

    const auto& mapping = loaded.mappings.front();
    expectEqual (mapping.id, "uniform.loudness", "mapping id");
    expectEqual (mapping.source.endpoint, "compound.loudness.out", "source endpoint");
    expectEqual (mapping.source.dataType, "signal.float", "source data type");
    expectEqual (mapping.source.streamKind, "continuous", "source stream kind");
    expectEqual (mapping.target.id, "shader.preview.main", "target id");
    expectEqual (mapping.target.kind, "shader.uniform", "target kind");
    expectEqual (mapping.target.uniformName, "u_loudness", "target uniform name");
    expectEqual (mapping.target.dataType, "signal.float", "target data type");

    const auto validation = myworld::validateGraphIOMapping (mapping);
    expect (validation.ok, validation.message);

    const auto binding = myworld::makeLiveIOBindingFromGraphIOMapping (mapping);
    expectEqual (binding.id, "uniform.loudness", "binding id");
    expectEqual (binding.sourceId, "compound.loudness.out", "binding source id");
    expect (binding.targetKind == myworld::LiveIOTargetKind::shaderUniform, "binding target kind");
    expectEqual (binding.uniformName, "u_loudness", "binding uniform name");

    myworld::LiveIOValueFrame frame;
    frame.values = {
        { "compound.loudness.out", 0.72, "compound.loudness.out" }
    };

    const auto report = myworld::evaluateGraphIOMapping (mapping, frame);
    expect (report.ok, report.message);
    expectEqual (report.status, "mapped", "graph io report status");
    expect (report.events.size() == 1, "one graph io event");
    expectEqual (report.events.front().sourceId, "compound.loudness.out", "event source id");
    expectEqual (report.events.front().uniformName, "u_loudness", "event uniform name");
    expectNear (report.events.front().inputValue, 0.72, 0.000001, "event input value");
    expectNear (report.events.front().normalizedValue, 0.72, 0.000001, "event normalized value");

    const auto json = myworld::makeGraphIOMappingReportJson (report);
    expectContains (json, "\"kind\": \"graphIOMappingReport\"", "report json kind");
    expectContains (json, "\"status\": \"mapped\"", "report json status");
    expectContains (json, "\"sourceEndpoint\": \"compound.loudness.out\"", "report json source");
    expectContains (json, "\"targetKind\": \"shader.uniform\"", "report json target");
    expectContains (json, "\"uniformName\": \"u_loudness\"", "report json uniform");
    expectContains (json, "\"inputValue\": 0.720000", "report json input value");
    expectContains (json, "\"normalizedValue\": 0.720000", "report json normalized value");

    const auto missing = myworld::evaluateGraphIOMapping (mapping, {});
    expect (! missing.ok, "missing source blocks graph io report");
    expectEqual (missing.status, "blocked", "missing source status");
    expect (missing.events.empty(), "missing source emits no events");
    expect (missing.diagnostics.size() == 1, "missing source diagnostic count");
    expectContains (missing.diagnostics.front(), "missing source: compound.loudness.out", "missing source diagnostic");

    auto invalid = mapping;
    invalid.target.uniformName.clear();
    const auto invalidValidation = myworld::validateGraphIOMapping (invalid);
    expect (! invalidValidation.ok, "invalid mapping fails validation");
    expectContains (invalidValidation.message, "shader uniform is required", "invalid mapping diagnostic");

    std::cout << "graph io mapping ok\n";
    return 0;
}
