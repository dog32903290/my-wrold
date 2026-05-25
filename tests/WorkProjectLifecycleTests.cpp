#include "WorkProjectLifecycle.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (condition)
        return;

    std::cerr << "FAIL: " << message << '\n';
    std::exit (1);
}

void expectLifecycle (myworld::WorkProjectLifecycleStatus status,
                      const std::string& expectedSource,
                      const std::string& expectedStatusText,
                      bool expectedBlocked)
{
    const auto lifecycle = myworld::makeWorkProjectLifecycle (status);
    expect (lifecycle.workSource == expectedSource, expectedStatusText + " source");
    expect (lifecycle.workSourceStatus == expectedStatusText, expectedStatusText + " status");
    expect (lifecycle.blocksSession == expectedBlocked, expectedStatusText + " blocked state");
}
}

int main()
{
    expectLifecycle (myworld::WorkProjectLifecycleStatus::activeWorkOpened,
                     "active-work",
                     "active-work-opened",
                     false);
    expectLifecycle (myworld::WorkProjectLifecycleStatus::activeWorkBlocked,
                     "active-work",
                     "active-work-blocked",
                     true);
    expectLifecycle (myworld::WorkProjectLifecycleStatus::fixtureFallbackNoActiveRequest,
                     "fixture",
                     "fixture-fallback-no-active-request",
                     false);
    expectLifecycle (myworld::WorkProjectLifecycleStatus::fixtureFallbackActiveMissing,
                     "fixture",
                     "fixture-fallback-active-missing",
                     false);
    expectLifecycle (myworld::WorkProjectLifecycleStatus::fixtureBlockedNoActiveRequest,
                     "fixture",
                     "fixture-blocked-no-active-request",
                     true);
    expectLifecycle (myworld::WorkProjectLifecycleStatus::fixtureBlockedActiveMissing,
                     "fixture",
                     "fixture-blocked-active-missing",
                     true);

    std::cout << "work project lifecycle ok\n";
    return 0;
}
