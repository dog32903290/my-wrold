#include "PathResolution.h"

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

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}
}

int main()
{
    const auto direct = myworld::resolvePathNearWithReport ({}, "fixtures/module-libraries/default.module-library.json");
    expect (direct.found, "direct fixture path resolves");
    expect (direct.resolvedPath == "fixtures/module-libraries/default.module-library.json", "direct fixture path remains stable");
    expect (! direct.attemptedPaths.empty(), "direct resolution records attempted path");

    const auto near = myworld::resolvePathNearWithReport ("fixtures/module-libraries/default.module-library.json",
                                                          "modules/loudness/module.json");
    expect (near.found, "near fixture path resolves from anchor parent walk");
    expectContains (near.resolvedPath, "fixtures/modules/loudness/module.json", "near fixture path");
    expect (near.attemptedPaths.size() >= 2, "near resolution records failed parent before success");

    const auto missing = myworld::resolvePathNearWithReport ("fixtures/module-libraries/default.module-library.json",
                                                             "modules/missing/module.json");
    expect (! missing.found, "missing fixture reports not found");
    expect (missing.resolvedPath == "modules/missing/module.json", "missing fixture keeps candidate fallback");
    expect (! missing.attemptedPaths.empty(), "missing fixture records attempted paths");

    const auto message = myworld::describePathResolutionFailure ("module package", missing);
    expectContains (message, "module package", "failure message");
    expectContains (message, "modules/missing/module.json", "failure message");
    expectContains (message, "attempted:", "failure message");

    expect (myworld::resolvePathNear ("fixtures/module-libraries/default.module-library.json",
                                      "modules/missing/module.json")
                == "modules/missing/module.json",
            "compatibility wrapper keeps missing candidate fallback");

    std::cout << "path resolution tests ok\n";
    return 0;
}
