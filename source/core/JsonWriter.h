#pragma once

#include <iosfwd>
#include <string>
#include <vector>

namespace myworld
{
std::string jsonEscaped (const std::string& text);
std::string jsonQuoted (const std::string& text);
void appendJsonStringArray (std::ostream& out, const std::vector<std::string>& values);
}
