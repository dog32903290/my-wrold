#pragma once

#include <string>

namespace myworld
{
enum class WorkProjectLifecycleStatus
{
    activeWorkOpened,
    activeWorkBlocked,
    fixtureFallbackNoActiveRequest,
    fixtureFallbackActiveMissing,
    fixtureBlockedNoActiveRequest,
    fixtureBlockedActiveMissing
};

struct WorkProjectLifecycle
{
    std::string workSource;
    std::string workSourceStatus;
    bool blocksSession = false;
};

WorkProjectLifecycle makeWorkProjectLifecycle (WorkProjectLifecycleStatus status);
}
