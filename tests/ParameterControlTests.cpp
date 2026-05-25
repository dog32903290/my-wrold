#include "GraphEndpoint.h"
#include "InteractionContract.h"
#include "ParameterControl.h"
#include "ParameterRowState.h"
#include "StorageContract.h"

#include <cstdlib>
#include <iostream>
#include <string>

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

const myworld::GraphNode& requireNode (const myworld::GraphContract& graph, const std::string& nodeId)
{
    const auto* node = myworld::findEditorNode (graph, nodeId);
    expect (node != nullptr, "missing node " + nodeId);
    return *node;
}

std::string paramValue (const myworld::GraphNode& node, const std::string& paramId)
{
    for (const auto& param : node.params)
        if (param.id == paramId)
            return param.value;

    return {};
}
}

int main()
{
    const myworld::ParamSpec floatSpec { "gain", "Gain", "float", "1.0", "0.0..8.0" };
    const auto floatControl = myworld::parameterControlForParam (floatSpec);
    expect (floatControl.kind == myworld::ParameterControlKind::floatSlider, "float range uses slider");
    expect (floatControl.hasRange, "float range parsed");
    expect (floatControl.minimum == 0.0 && floatControl.maximum == 8.0, "float range bounds");

    auto normalized = myworld::normalizeParameterEdit (floatSpec, "9.2500");
    expect (normalized.ok, normalized.error);
    expect (normalized.value == "8", "float edit clamps and trims");
    normalized = myworld::normalizeParameterEdit (floatSpec, "2.5000");
    expect (normalized.ok && normalized.value == "2.5", "float edit normalizes decimals");

    const myworld::ParamSpec intSpec { "channel", "Channel", "int", "1", "1..16" };
    const auto intControl = myworld::parameterControlForParam (intSpec);
    expect (intControl.kind == myworld::ParameterControlKind::integerStepper, "int range uses stepper");
    normalized = myworld::normalizeParameterEdit (intSpec, "17");
    expect (normalized.ok && normalized.value == "16", "int edit clamps high");
    normalized = myworld::normalizeParameterEdit (intSpec, "3.5");
    expect (! normalized.ok, "int edit rejects fractional text");

    const myworld::ParamSpec boolSpec { "enabled", "Enabled", "bool", "false", "" };
    expect (myworld::parameterControlForParam (boolSpec).kind == myworld::ParameterControlKind::toggle,
            "bool uses toggle");
    expect (myworld::normalizeParameterEdit (boolSpec, "on").value == "true", "bool accepts on");
    expect (myworld::normalizeParameterEdit (boolSpec, "0").value == "false", "bool accepts 0");

    const myworld::ParamSpec vecSpec { "offset", "Offset", "vec3", "0,0,0", "-1.0..1.0" };
    const auto vecControl = myworld::parameterControlForParam (vecSpec);
    expect (vecControl.kind == myworld::ParameterControlKind::vectorEditor, "vec3 uses vector editor");
    expect (vecControl.componentCount == 3, "vec3 component count");
    normalized = myworld::normalizeParameterEdit (vecSpec, "1, 0.5000, -2");
    expect (normalized.ok && normalized.value == "1,0.5,-1", "vec3 normalizes and clamps components");
    normalized = myworld::normalizeParameterEdit (vecSpec, "1, 2");
    expect (! normalized.ok, "vec3 rejects missing component");

    const myworld::ParamSpec enumSpec { "mode", "Mode", "enum", "soft", "soft|hard|bypass" };
    const auto enumControl = myworld::parameterControlForParam (enumSpec);
    expect (enumControl.kind == myworld::ParameterControlKind::enumMenu, "enum uses menu");
    expect (enumControl.options.size() == 3, "enum options parsed");
    expect (myworld::normalizeParameterEdit (enumSpec, "hard").value == "hard", "enum accepts option");
    expect (! myworld::normalizeParameterEdit (enumSpec, "missing").ok, "enum rejects unknown option");

    const myworld::ParamSpec stringSpec { "device", "Device", "string", "system", "" };
    expect (myworld::parameterControlForParam (stringSpec).kind == myworld::ParameterControlKind::textField,
            "string uses text field");
    expect (myworld::normalizeParameterEdit (stringSpec, "Built-in Input").value == "Built-in Input",
            "string preserves text");

    const myworld::ParamSpec pathSpec { "file", "File", "resource.file", "", "" };
    expect (myworld::parameterControlForParam (pathSpec).kind == myworld::ParameterControlKind::pathField,
            "resource file uses path field");

    const myworld::ParamSpec glslSpec { "source", "Source", "text.glsl", "", "" };
    expect (myworld::parameterControlForParam (glslSpec).kind == myworld::ParameterControlKind::multilineText,
            "GLSL uses multiline text");
    expect (myworld::normalizeParameterEdit (glslSpec, "void main(){}\n").value == "void main(){}\n",
            "GLSL preserves multiline text");

    auto session = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    expect (myworld::createNode (session, "analyzer.analysis_gain", "gain1", { 180.0, 240.0 }).ok,
            "create gain");

    const auto typedSet = myworld::setTypedParam (session, "gain1", floatSpec, "9.25");
    expect (typedSet.ok, typedSet.message);
    expect (session.commandLog.back() == "set_param", "typed set uses set_param command");
    expect (paramValue (requireNode (session.graph, "gain1"), "gain") == "8",
            "typed set stores normalized value");

    const auto invalidSet = myworld::setTypedParam (session, "gain1", intSpec, "not an int");
    expect (! invalidSet.ok, "invalid typed set rejected");
    expect (paramValue (requireNode (session.graph, "gain1"), "channel").empty(),
            "invalid typed set does not mutate graph");

    const auto document = myworld::makePatchDocument ("patch.param-control", "Param Control", session.graph);
    const auto parsed = myworld::parsePatchDocument (myworld::toJson (document));
    expect (parsed.ok, parsed.error);
    expect (paramValue (requireNode (parsed.document.graph, "gain1"), "gain") == "8",
            "typed value roundtrips through PatchDocument");

    std::cout << "parameter controls ok\n";
    return 0;
}
