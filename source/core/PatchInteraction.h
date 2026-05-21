#pragma once

#include <string>

namespace myworld
{
bool isKnownPatchGesture (const std::string& gesture);
std::string commandForGesture (const std::string& gesture);
}
