#pragma once

#include "InteractionContract.h"

#include <string>
#include <vector>

namespace myworld
{
enum class ParameterControlKind
{
    floatSlider,
    integerStepper,
    toggle,
    vectorEditor,
    enumMenu,
    textField,
    multilineText,
    pathField,
    unsupported
};

struct ParameterControlState
{
    ParameterControlKind kind = ParameterControlKind::unsupported;
    std::string dataType;
    bool hasRange = false;
    double minimum = 0.0;
    double maximum = 0.0;
    int componentCount = 0;
    std::vector<std::string> options;
};

struct ParameterEditResult
{
    bool ok = false;
    std::string value;
    std::string error;
};

ParameterControlState parameterControlForParam (const ParamSpec& param);
ParameterEditResult normalizeParameterEdit (const ParamSpec& param, const std::string& rawValue);
CommandResult setTypedParam (GraphSession& session,
                             const std::string& nodeId,
                             const ParamSpec& param,
                             const std::string& rawValue);
}
