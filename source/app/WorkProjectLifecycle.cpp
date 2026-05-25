#include "WorkProjectLifecycle.h"

namespace myworld
{
WorkProjectLifecycle makeWorkProjectLifecycle (WorkProjectLifecycleStatus status)
{
    switch (status)
    {
        case WorkProjectLifecycleStatus::activeWorkOpened:
            return { "active-work", "active-work-opened", false };

        case WorkProjectLifecycleStatus::activeWorkBlocked:
            return { "active-work", "active-work-blocked", true };

        case WorkProjectLifecycleStatus::fixtureFallbackNoActiveRequest:
            return { "fixture", "fixture-fallback-no-active-request", false };

        case WorkProjectLifecycleStatus::fixtureFallbackActiveMissing:
            return { "fixture", "fixture-fallback-active-missing", false };

        case WorkProjectLifecycleStatus::fixtureBlockedNoActiveRequest:
            return { "fixture", "fixture-blocked-no-active-request", true };

        case WorkProjectLifecycleStatus::fixtureBlockedActiveMissing:
            return { "fixture", "fixture-blocked-active-missing", true };
    }

    return { "fixture", "fixture-blocked-no-active-request", true };
}
}
