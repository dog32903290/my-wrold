#include "InteractionContract.h"
#include "GraphEndpoint.h"
#include "ParameterRowState.h"
#include "VariationState.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}

const myworld::ParameterRowState* findRow (const std::vector<myworld::ParameterRowState>& rows,
                                           const std::string& rowId)
{
    for (const auto& row : rows)
        if (row.id == rowId)
            return &row;

    return nullptr;
}

const myworld::ParameterRowState& requireRow (const std::vector<myworld::ParameterRowState>& rows,
                                             const std::string& rowId)
{
    const auto* row = findRow (rows, rowId);
    expect (row != nullptr, "missing row " + rowId);
    return *row;
}

bool hasValue (const myworld::VariationRecord& record, const std::string& paramId)
{
    for (const auto& value : record.values)
        if (value.paramId == paramId)
            return true;

    return false;
}

bool hasSkipReason (const myworld::VariationRecord& record,
                    const std::string& paramId,
                    myworld::VariationSkipReason reason)
{
    for (const auto& skipped : record.skippedValues)
        if (skipped.paramId == paramId && skipped.reason == reason)
            return true;

    return false;
}
}

int main()
{
    myworld::NodeSpec spec;
    spec.type = "test.metadata";
    spec.displayName = "Metadata Test";

    myworld::ParamSpec mode { "mode", "Mode", "enum", "simple", "simple|advanced" };
    mode.group = "General";
    mode.description = "Chooses which detail controls are relevant.";

    myworld::ParamSpec detail { "detail", "Detail", "float", "0.5", "0.0..1.0" };
    detail.group = "Advanced";
    detail.description = "Only relevant in advanced mode.";
    detail.visibleWhenParamId = "mode";
    detail.visibleWhenValue = "advanced";

    myworld::ParamSpec secret { "secret", "Secret", "string", "", "" };
    secret.group = "Debug";
    secret.description = "Diagnostic note that should not enter presets.";
    secret.excludeFromPresets = true;

    spec.params = { mode, detail, secret };

    auto session = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    const std::vector<myworld::NodeSpec> specs { spec };
    expect (myworld::createNode (session, specs, "test.metadata", "meta1", { 120.0, 180.0 }).ok,
            "create metadata node");

    auto rows = myworld::parameterRowsForNode (session.graph,
                                               *myworld::findEditorNode (session.graph, "meta1"),
                                               spec);
    const auto& modeRow = requireRow (rows, "param.mode");
    expect (modeRow.group == "General", "row exposes group");
    expect (modeRow.description == mode.description, "row exposes description");
    expect (! modeRow.excludeFromPresets, "ordinary row is preset-capturable");
    expect (findRow (rows, "param.detail") == nullptr, "irrelevant row is hidden by default value");

    expect (myworld::setParam (session, "meta1", "mode", "advanced").ok, "set mode advanced");
    expect (myworld::setParam (session, "meta1", "detail", "0.8").ok, "set detail");
    expect (myworld::setParam (session, "meta1", "secret", "keep-out").ok, "set secret");

    rows = myworld::parameterRowsForNode (session.graph,
                                          *myworld::findEditorNode (session.graph, "meta1"),
                                          spec);
    const auto& detailRow = requireRow (rows, "param.detail");
    expect (detailRow.group == "Advanced", "visible row keeps group");
    expect (detailRow.description == detail.description, "visible row keeps description");
    expect (requireRow (rows, "param.secret").excludeFromPresets, "row exposes preset exclusion");

    expect (myworld::createPreset (session, "meta1", spec, "preset.metadata", "Metadata", {}).ok,
            "create metadata preset");
    const auto& preset = session.variations.presets.front();
    expect (hasValue (preset, "mode"), "preset captures ordinary metadata param");
    expect (hasValue (preset, "detail"), "preset captures relevant non-default param");
    expect (! hasValue (preset, "secret"), "preset omits ParamSpec-excluded param");
    expect (hasSkipReason (preset, "secret", myworld::VariationSkipReason::excludedFromPresets),
            "preset records ParamSpec exclusion reason");

    std::cout << "parameter metadata ok\n";
    return 0;
}
