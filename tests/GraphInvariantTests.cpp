#include "GraphContract.h"
#include "InteractionContract.h"
#include "NodeSpec.h"

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
}

int main()
{
    const auto specs = myworld::makeSeedNodeSpecs();
    auto graph = myworld::makeDefaultShaderOutputGraph();

    auto report = myworld::validateGraphInvariants (graph, specs);
    expect (report.ok, "default graph validates");

    auto dangling = graph;
    dangling.editorGraph.edges[0].to = "missing.input";
    report = myworld::validateGraphInvariants (dangling, specs);
    expect (! report.ok, "dangling target rejected");
    expect (! report.errors.empty(), "dangling target reports error");

    auto wrongType = graph;
    wrongType.editorGraph.edges[0].from = "out1.output";
    wrongType.editorGraph.edges[0].to = "shader1.input";
    report = myworld::validateGraphInvariants (wrongType, specs);
    expect (! report.ok, "wrong port direction rejected");

    auto duplicateInput = graph;
    duplicateInput.editorGraph.nodes.push_back ({ "shader2", "shader.fragment", { "u_time" }, { 240.0, 80.0 } });
    duplicateInput.editorGraph.edges.push_back ({ "shader2.output", "out1.input", "edge.shader2.output.out1.input", "texture.rgba", "continuous" });
    report = myworld::validateGraphInvariants (duplicateInput, specs);
    expect (! report.ok, "single input cardinality rejected");

    std::cout << "graph invariants ok\n";
    return 0;
}
