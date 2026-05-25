#include "ShaderPreviewInputBridge.h"

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

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}
}

int main()
{
    const auto snapshot = myworld::makeShaderPreviewInputFromUniformEvidence (
        "uniform.loudness",
        "u_loudness",
        0.5,
        64);

    expect (snapshot.ok, snapshot.message);
    expectEqual (snapshot.status, "ready", "snapshot status");
    expectEqual (snapshot.uniforms.front().bindingId, "uniform.loudness", "binding id");
    expectEqual (snapshot.uniforms.front().uniformName, "u_loudness", "uniform name");
    expect (snapshot.uniforms.front().value > 0.499 && snapshot.uniforms.front().value < 0.501,
            "uniform value");
    expect (snapshot.sampleCounter == 64, "sample counter");

    const auto missing = myworld::makeShaderPreviewInputFromUniformEvidence (
        "uniform.loudness",
        "",
        0.5,
        64);
    expect (! missing.ok, "missing uniform is rejected");
    expectEqual (missing.status, "failed", "missing status");

    const auto json = myworld::makeShaderPreviewInputSnapshotJson (snapshot);
    expectContains (json, "\"kind\": \"shaderPreviewInputSnapshot\"", "json kind");
    expectContains (json, "\"status\": \"ready\"", "json status");
    expectContains (json, "\"bindingId\": \"uniform.loudness\"", "json binding");
    expectContains (json, "\"uniformName\": \"u_loudness\"", "json uniform");
    expectContains (json, "\"value\": 0.500000", "json value");
    expectContains (json, "\"sampleCounter\": 64", "json sample");

    std::cout << "shader preview input bridge ok\n";
    return 0;
}
