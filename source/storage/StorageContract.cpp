#include "StorageContract.h"

#include <algorithm>
#include <array>

namespace myworld
{
PatchDocumentManifest makeMinimalPatchDocument (const std::string& id, const std::string& title)
{
    return { id, title, "editorGraph", "runtimeGraph", "portBindings" };
}

bool isKnownSaveStatus (const std::string& status)
{
    static constexpr std::array<const char*, 6> statuses {
        "clean",
        "saved-and-committed",
        "save-ok commit-pending",
        "save-ok commit-failed",
        "validation-failed",
        "write-failed"
    };

    return std::find (statuses.begin(), statuses.end(), status) != statuses.end();
}
}
