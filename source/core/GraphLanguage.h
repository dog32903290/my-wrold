#pragma once

#include <string>

namespace myworld
{
struct TypedEdge
{
    std::string id;
    std::string from;
    std::string to;
    std::string dataType;
    std::string streamKind;
};

struct PortBinding
{
    std::string id;
    std::string dataType;
    std::string bindingMode;
};

bool isKnownRegionType (const std::string& type);
bool isKnownTypeSpec (const std::string& type);
bool isKnownStreamKind (const std::string& kind);
bool isKnownPortBindingMode (const std::string& mode);
bool isKnownCommandType (const std::string& type);
bool isValidTypedEdge (const TypedEdge& edge);
bool isValidPortBinding (const PortBinding& binding);
bool isAllowedCompilerWorkerLanguage (const std::string& language);
bool isAllowedRealtimeRuntimeLanguage (const std::string& language);
}
