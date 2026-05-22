#include "InteractionContract.h"

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
    const auto report = myworld::runBehaviorTraceFixture ("fixtures/interaction/tooll3-t0-t7.behavior.json");
    expect (report.ok, report.errors.empty() ? "trace report ok" : report.errors.front());
    expect (report.tracesRun == 8, "eight traces run");
    expect (report.commandsObserved.size() >= 16, "commands observed");

    std::cout << "interaction traces ok\n";
    return 0;
}
