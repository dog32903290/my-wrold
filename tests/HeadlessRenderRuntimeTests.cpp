#include "HeadlessRenderRuntime.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
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

std::string readText (const std::filesystem::path& path)
{
    std::ifstream file { path };
    std::ostringstream text;
    text << file.rdbuf();
    return text.str();
}

void expectFileContains (const std::filesystem::path& path,
                         const std::string& expected,
                         const std::string& message)
{
    expect (std::filesystem::exists (path), message + " file exists");
    const auto text = readText (path);
    expect (text.find (expected) != std::string::npos,
            message + " should contain " + expected);
}

std::string invalidResolutionFixture()
{
    return R"({
  "kind": "runtimeProofFixture",
  "version": 1,
  "name": "invalid_resolution",
  "graph": {
    "nodes": [
      {
        "id": "const1",
        "type": "image.constant",
        "params": {
          "color": [0.02, 0.02, 0.02, 1.0],
          "resolution": [0, 720]
        }
      },
      {
        "id": "out1",
        "type": "output.texture_summary",
        "params": {
          "path": "debug/r2-top-constant-invalid/texture_summary.json"
        }
      }
    ],
    "edges": [
      {
        "id": "edge1",
        "from": "const1.out",
        "to": "out1.input",
        "dataType": "texture.rgba",
        "streamKind": "continuous"
      }
    ]
  }
})";
}

std::string unknownNodeTypeFixture()
{
    return R"({
  "kind": "runtimeProofFixture",
  "version": 1,
  "name": "unknown_type",
  "graph": {
    "nodes": [
      {
        "id": "noise1",
        "type": "image.noise",
        "params": {
          "resolution": [1280, 720]
        }
      },
      {
        "id": "out1",
        "type": "output.texture_summary",
        "params": {
          "path": "debug/r2-top-constant-unknown/texture_summary.json"
        }
      }
    ],
    "edges": [
      {
        "id": "edge1",
        "from": "noise1.out",
        "to": "out1.input",
        "dataType": "texture.rgba",
        "streamKind": "continuous"
      }
    ]
  }
})";
}
}

int main()
{
    const std::filesystem::path outputDirectory { "debug/r2-top-constant" };
    std::filesystem::remove_all (outputDirectory);

    const auto result = myworld::runHeadlessRenderRuntimeProof (
        "fixtures/runtime/top_constant_to_output.graph.json",
        outputDirectory.string());

    expect (result.ok, result.error);
    expect (result.textureSummaryPath == "debug/r2-top-constant/texture_summary.json",
            "texture summary path");
    expect (result.cookOrderPath == "debug/r2-top-constant/cook_order.json",
            "cook order path");
    expect (result.nodeStatsPath == "debug/r2-top-constant/node_stats.json",
            "node stats path");
    expect (result.errorsPath == "debug/r2-top-constant/errors.json",
            "errors path");

    expectFileContains (outputDirectory / "texture_summary.json",
                        "\"width\": 1280",
                        "texture summary");
    expectFileContains (outputDirectory / "texture_summary.json",
                        "\"height\": 720",
                        "texture summary");
    expectFileContains (outputDirectory / "texture_summary.json",
                        "\"format\": \"rgba8\"",
                        "texture summary");
    expectFileContains (outputDirectory / "texture_summary.json",
                        "\"color\": [0.020000, 0.020000, 0.020000, 1.000000]",
                        "texture summary");
    expectFileContains (outputDirectory / "cook_order.json",
                        "\"cookOrder\": [\"const1\", \"out1\"]",
                        "cook order");
    expectFileContains (outputDirectory / "cook_order.json",
                        "\"version\": 1",
                        "cook order");
    expectFileContains (outputDirectory / "node_stats.json",
                        "\"version\": 1",
                        "node stats");
    expectFileContains (outputDirectory / "node_stats.json",
                        "\"renderer\": \"headless\"",
                        "node stats");
    expectFileContains (outputDirectory / "node_stats.json",
                        "\"type\": \"image.constant\"",
                        "node stats");
    expectFileContains (outputDirectory / "node_stats.json",
                        "\"cookDomain\": \"render\"",
                        "node stats");
    expectFileContains (outputDirectory / "errors.json",
                        "\"ok\": true",
                        "success errors");

    const std::filesystem::path invalidDirectory { "debug/r2-top-constant-invalid-resolution" };
    std::filesystem::remove_all (invalidDirectory);
    const auto invalidResult = myworld::runHeadlessRenderRuntimeProofText (
        invalidResolutionFixture(),
        invalidDirectory.string());
    expect (! invalidResult.ok, "invalid resolution fails");
    expect (invalidResult.error.find ("invalid resolution") != std::string::npos,
            "invalid resolution error");
    expectFileContains (invalidDirectory / "errors.json",
                        "invalid resolution",
                        "invalid resolution errors");

    const std::filesystem::path unknownDirectory { "debug/r2-top-constant-unknown-type" };
    std::filesystem::remove_all (unknownDirectory);
    const auto unknownResult = myworld::runHeadlessRenderRuntimeProofText (
        unknownNodeTypeFixture(),
        unknownDirectory.string());
    expect (! unknownResult.ok, "unknown node type fails");
    expect (unknownResult.error.find ("unsupported node type") != std::string::npos,
            "unknown node type error");
    expectFileContains (unknownDirectory / "errors.json",
                        "unsupported node type",
                        "unknown node type errors");

    std::cout << "headless render runtime ok\n";
    return 0;
}
