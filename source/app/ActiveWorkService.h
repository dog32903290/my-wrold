#pragma once

#include "InteractionContract.h"
#include "StorageCommand.h"

#include <string>

namespace myworld
{
CommandResult saveActiveWorkProject (GraphSession& session);
PublishModuleResult publishSelectedModuleFromActiveWork (GraphSession& session, const std::string& sourceNodeId);
}
