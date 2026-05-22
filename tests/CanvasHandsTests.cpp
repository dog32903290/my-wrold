#include "CanvasHands.h"

#include <algorithm>
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

bool contains (const std::vector<std::string>& values, const std::string& value)
{
    return std::find (values.begin(), values.end(), value) != values.end();
}
}

int main()
{
    const auto specs = myworld::makeSeedNodeSpecs();
    const myworld::CanvasHandViewport viewport { 640.0, 360.0 };

    auto session = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    session.view = { 1.25, 18.0, 12.0 };

    const auto resolved = myworld::resolveCanvasHandTarget (session, specs, viewport, myworld::canvasHandPort ("shader1.output"));
    expect (resolved.ok, resolved.message);
    expect (resolved.hit.kind == myworld::HitTestKind::outputPort, "resolved port hit kind");
    expect (resolved.hit.endpoint == "shader1.output", "resolved port endpoint");
    expect (resolved.screenPoint.x >= 0.0 && resolved.screenPoint.x <= viewport.width, "resolved x inside viewport");
    expect (resolved.screenPoint.y >= 0.0 && resolved.screenPoint.y <= viewport.height, "resolved y inside viewport");

    auto moveSession = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    const auto moveReport = myworld::runCanvasHandTrace (moveSession,
                                                         specs,
                                                         viewport,
                                                         { myworld::canvasHandDrag (myworld::canvasHandNode ("shader1"),
                                                                                   myworld::canvasHandPoint ({ 150.0, 120.0 })) });
    expect (moveReport.ok, moveReport.errors.empty() ? "move report" : moveReport.errors.front());
    expect (contains (moveReport.commandLogDelta, "move_node"), "move command observed");
    expect (contains (moveReport.stateLog, "drag:left:nodeBody->nodeBody"), "move state log");

    auto connectSession = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    connectSession.graph.editorGraph.edges.clear();
    connectSession.graph.runtimeGraph.edges.clear();

    const auto connectReport = myworld::runCanvasHandTrace (connectSession,
                                                            specs,
                                                            viewport,
                                                            { myworld::canvasHandDrag (myworld::canvasHandPort ("shader1.output"),
                                                                                      myworld::canvasHandPort ("out1.input")) });
    expect (connectReport.ok, connectReport.errors.empty() ? "connect report" : connectReport.errors.front());
    expect (contains (connectReport.commandLogDelta, "connect"), "connect command observed");
    expect (connectSession.graph.editorGraph.edges.size() == 1, "edge created");
    expect (connectSession.graph.editorGraph.edges.front().from == "shader1.output", "edge source");
    expect (connectSession.graph.editorGraph.edges.front().to == "out1.input", "edge target");

    auto clickSession = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    const auto clickReport = myworld::runCanvasHandTrace (clickSession,
                                                          specs,
                                                          viewport,
                                                          { myworld::canvasHandClick (myworld::canvasHandNode ("shader1"),
                                                                                     myworld::CanvasHandButton::left) });
    expect (clickReport.ok, clickReport.errors.empty() ? "click report" : clickReport.errors.front());
    expect (clickReport.commandLogDelta.empty(), "click selection does not mutate graph commands");
    expect (clickSession.selectedNodeIds.size() == 1 && clickSession.selectedNodeIds.front() == "shader1", "click selects node");
    expect (contains (clickReport.stateLog, "click:left:nodeBody"), "click state log");

    auto pointerSession = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    const auto pointerReport = myworld::runCanvasHandTrace (pointerSession,
                                                            specs,
                                                            viewport,
                                                            { myworld::canvasHandMoveTo (myworld::canvasHandNode ("shader1")),
                                                              myworld::canvasHandDown (myworld::CanvasHandButton::right),
                                                              myworld::canvasHandUp (myworld::CanvasHandButton::right),
                                                              myworld::canvasHandDown (myworld::CanvasHandButton::middle),
                                                              myworld::canvasHandUp (myworld::CanvasHandButton::middle),
                                                              myworld::canvasHandWheel (myworld::canvasHandViewportCenter(), 0.0, -120.0) });
    expect (pointerReport.ok, pointerReport.errors.empty() ? "pointer report" : pointerReport.errors.front());
    expect (pointerReport.commandLogDelta.empty(), "pointer-only actions do not mutate graph");
    expect (contains (pointerReport.stateLog, "down:right"), "right down logged");
    expect (contains (pointerReport.stateLog, "up:right"), "right up logged");
    expect (contains (pointerReport.stateLog, "down:middle"), "middle down logged");
    expect (contains (pointerReport.stateLog, "wheel:0,-120"), "wheel logged");

    std::cout << "canvas hands ok\n";
    return 0;
}
