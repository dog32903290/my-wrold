#include "NodeSpecQueries.h"

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

void expectEqual (const std::string& actual, const std::string& expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + expected + " got " + actual);
}
}

int main()
{
    const auto specs = myworld::makeSeedNodeSpecs();
    const auto graph = myworld::makeDefaultShaderOutputGraph();

    expectEqual (myworld::outputDataTypeForEndpoint (graph, specs, "shader1.output"),
                 "texture.rgba",
                 "output endpoint data type");
    expectEqual (myworld::outputDataTypeForEndpoint (graph, specs, "out1.input"),
                 "",
                 "input endpoint has no output data type");
    expectEqual (myworld::outputDataTypeForEndpoint (graph, specs, "missing.output"),
                 "",
                 "missing endpoint has no data type");

    const auto* loudness = myworld::findNodeSpec (specs, "analyzer.loudness");
    expect (loudness != nullptr, "loudness spec exists");
    expect (myworld::canCreateFromEndpoint (*loudness, "audio.mono"), "compatible create from audio mono");
    expect (! myworld::canCreateFromEndpoint (*loudness, "texture.rgba"), "incompatible create from texture");
    expect (myworld::nodeSpecMatchesFilter (*loudness, "feature"), "filter matches category text");
    expect (! myworld::nodeSpecMatchesFilter (*loudness, "definitely missing"), "filter rejects missing text");
    expectEqual (myworld::primaryDataTypeForSpec (loudness), "signal.float", "primary output data type");

    expectEqual (myworld::makeNodeIdStem ("  Compound.Loudness Missing RuntimeOp! "),
                 "compound_loudness_missing_runtimeop",
                 "node id stem normalizes punctuation");

    auto graphWithExisting = graph;
    myworld::GraphNode existingTexture;
    existingTexture.id = "texture_noise1";
    existingTexture.type = "image.texture";
    graphWithExisting.editorGraph.nodes.push_back (existingTexture);
    expectEqual (myworld::makeUniqueNodeId (graphWithExisting, "texture.noise"),
                 "texture_noise2",
                 "unique node id skips existing stem");

    std::cout << "node spec query tests ok\n";
    return 0;
}
