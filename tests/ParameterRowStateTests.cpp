#include "GraphEndpoint.h"
#include "InteractionContract.h"
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

const myworld::NodeSpec& requireSpec (const std::vector<myworld::NodeSpec>& specs, const std::string& nodeType)
{
    const auto* spec = myworld::findNodeSpec (specs, nodeType);
    expect (spec != nullptr, "missing spec " + nodeType);
    return *spec;
}

const myworld::ParameterRowState& requireRow (const std::vector<myworld::ParameterRowState>& rows,
                                             const std::string& rowId)
{
    for (const auto& row : rows)
        if (row.id == rowId)
            return row;

    expect (false, "missing row " + rowId);
    return rows.front();
}
}

int main()
{
    const auto specs = myworld::makeSeedNodeSpecs();
    auto session = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());

    expect (myworld::createNode (session, specs, "analyzer.analysis_gain", "gain1", { 180.0, 260.0 }).ok,
            "create gain node");
    auto rows = myworld::parameterRowsForNode (session.graph,
                                               requireNode (session.graph, "gain1"),
                                               requireSpec (specs, "analyzer.analysis_gain"));
    const auto& defaultGain = requireRow (rows, "param.gain");
    expect (defaultGain.valueState == myworld::ParameterRowValueState::defaultValue, "gain starts default");
    expect (defaultGain.value == "1.0", "gain default value");
    expect (defaultGain.stateLabel == "default", "gain default label");

    expect (myworld::setParam (session, "gain1", "gain", "2.5").ok, "set gain manual");
    rows = myworld::parameterRowsForNode (session.graph,
                                          requireNode (session.graph, "gain1"),
                                          requireSpec (specs, "analyzer.analysis_gain"));
    const auto& manualGain = requireRow (rows, "param.gain");
    expect (manualGain.valueState == myworld::ParameterRowValueState::manual, "gain becomes manual");
    expect (manualGain.value == "2.5", "gain manual value");

    expect (myworld::resetParam (session, "gain1", "gain").ok, "reset gain");
    rows = myworld::parameterRowsForNode (session.graph,
                                          requireNode (session.graph, "gain1"),
                                          requireSpec (specs, "analyzer.analysis_gain"));
    expect (requireRow (rows, "param.gain").valueState == myworld::ParameterRowValueState::defaultValue,
            "reset returns gain to default");
    expect (session.commandLog.back() == "reset_param", "reset param command logged");
    expect (myworld::undo (session), "undo reset param");
    rows = myworld::parameterRowsForNode (session.graph,
                                          requireNode (session.graph, "gain1"),
                                          requireSpec (specs, "analyzer.analysis_gain"));
    expect (requireRow (rows, "param.gain").value == "2.5", "undo reset restores manual value");
    expect (myworld::redo (session), "redo reset param");
    rows = myworld::parameterRowsForNode (session.graph,
                                          requireNode (session.graph, "gain1"),
                                          requireSpec (specs, "analyzer.analysis_gain"));
    expect (requireRow (rows, "param.gain").valueState == myworld::ParameterRowValueState::defaultValue,
            "redo reset restores default");

    rows = myworld::parameterRowsForNode (session.graph,
                                          requireNode (session.graph, "out1"),
                                          requireSpec (specs, "output.preview"));
    const auto& connectedInput = requireRow (rows, "input.input");
    expect (connectedInput.valueState == myworld::ParameterRowValueState::connected,
            "edge-connected input row is connected");
    expect (connectedInput.value == "shader1.output", "connected input value names source endpoint");

    expect (myworld::disconnectEdge (session, "edge.shader1.output.out1.input").ok, "disconnect output input");
    rows = myworld::parameterRowsForNode (session.graph,
                                          requireNode (session.graph, "out1"),
                                          requireSpec (specs, "output.preview"));
    expect (requireRow (rows, "input.input").valueState == myworld::ParameterRowValueState::defaultValue,
            "disconnected input row returns to default");

    expect (myworld::setPortBinding (session, "out1", "input", "animated", "timeline.opacity").ok,
            "set animated input binding");
    rows = myworld::parameterRowsForNode (session.graph,
                                          requireNode (session.graph, "out1"),
                                          requireSpec (specs, "output.preview"));
    const auto& animatedInput = requireRow (rows, "input.input");
    expect (animatedInput.valueState == myworld::ParameterRowValueState::animated, "input becomes animated");
    expect (animatedInput.value == "timeline.opacity", "animated input value");

    expect (myworld::resetPortBinding (session, "out1", "input").ok, "reset input binding");
    expect (session.commandLog.back() == "reset_port_binding", "reset binding command logged");
    rows = myworld::parameterRowsForNode (session.graph,
                                          requireNode (session.graph, "out1"),
                                          requireSpec (specs, "output.preview"));
    expect (requireRow (rows, "input.input").valueState == myworld::ParameterRowValueState::defaultValue,
            "reset input binding returns to default");

    expect (myworld::setParam (session, "gain1", "gain", "3.0").ok, "set gain for document roundtrip");
    expect (myworld::setPortBinding (session, "out1", "input", "manual", "manual.texture").ok,
            "set manual binding for document roundtrip");
    const auto document = myworld::makePatchDocument ("patch.param-row", "Param Rows", session.graph);
    const auto parsed = myworld::parsePatchDocument (myworld::toJson (document));
    expect (parsed.ok, parsed.error);

    rows = myworld::parameterRowsForNode (parsed.document.graph,
                                          requireNode (parsed.document.graph, "gain1"),
                                          requireSpec (specs, "analyzer.analysis_gain"));
    expect (requireRow (rows, "param.gain").valueState == myworld::ParameterRowValueState::manual,
            "manual param row roundtrips");
    expect (requireRow (rows, "param.gain").value == "3.0", "manual param value roundtrips");

    rows = myworld::parameterRowsForNode (parsed.document.graph,
                                          requireNode (parsed.document.graph, "out1"),
                                          requireSpec (specs, "output.preview"));
    expect (requireRow (rows, "input.input").valueState == myworld::ParameterRowValueState::manual,
            "manual input binding roundtrips");
    expect (requireRow (rows, "input.input").value == "manual.texture", "manual input value roundtrips");

    std::cout << "parameter row state ok\n";
    return 0;
}
