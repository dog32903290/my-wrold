#include "ParameterControl.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <sstream>

namespace myworld
{
namespace
{
std::string trim (std::string text)
{
    while (! text.empty() && std::isspace (static_cast<unsigned char> (text.back())) != 0)
        text.pop_back();

    auto first = text.begin();
    while (first != text.end() && std::isspace (static_cast<unsigned char> (*first)) != 0)
        ++first;

    text.erase (text.begin(), first);
    return text;
}

std::string lowercased (std::string text)
{
    std::transform (text.begin(), text.end(), text.begin(), [] (unsigned char character) {
        return static_cast<char> (std::tolower (character));
    });
    return text;
}

std::vector<std::string> split (const std::string& text, char delimiter)
{
    std::vector<std::string> values;
    std::string item;
    std::istringstream in (text);

    while (std::getline (in, item, delimiter))
        values.push_back (trim (item));

    return values;
}

bool parseDoubleStrict (const std::string& text, double& value)
{
    const auto trimmed = trim (text);
    if (trimmed.empty())
        return false;

    errno = 0;
    char* end = nullptr;
    const auto parsed = std::strtod (trimmed.c_str(), &end);
    if (end == trimmed.c_str() || *end != '\0' || errno == ERANGE || ! std::isfinite (parsed))
        return false;

    value = parsed;
    return true;
}

bool parseIntStrict (const std::string& text, int& value)
{
    const auto trimmed = trim (text);
    if (trimmed.empty())
        return false;

    size_t index = 0;
    if (trimmed[index] == '-' || trimmed[index] == '+')
        ++index;

    if (index >= trimmed.size())
        return false;

    for (; index < trimmed.size(); ++index)
        if (std::isdigit (static_cast<unsigned char> (trimmed[index])) == 0)
            return false;

    errno = 0;
    char* end = nullptr;
    const auto parsed = std::strtol (trimmed.c_str(), &end, 10);
    if (*end != '\0' || errno == ERANGE)
        return false;

    value = static_cast<int> (parsed);
    return true;
}

std::string formatDouble (double value)
{
    if (std::abs (value) < 0.000000000001)
        value = 0.0;

    std::ostringstream out;
    out << std::setprecision (12) << value;
    return out.str();
}

bool parseRange (const std::string& text, double& minimum, double& maximum)
{
    const auto divider = text.find ("..");
    if (divider == std::string::npos)
        return false;

    if (! parseDoubleStrict (text.substr (0, divider), minimum)
        || ! parseDoubleStrict (text.substr (divider + 2), maximum))
    {
        return false;
    }

    if (minimum > maximum)
        std::swap (minimum, maximum);

    return true;
}

std::vector<std::string> enumOptions (const ParamSpec& param)
{
    if (param.range.find ('|') == std::string::npos)
        return {};

    auto options = split (param.range, '|');
    options.erase (std::remove_if (options.begin(), options.end(), [] (const auto& option) {
        return option.empty();
    }), options.end());
    return options;
}

int vectorComponentCount (const std::string& dataType)
{
    if (dataType == "vec2" || dataType == "float2")
        return 2;

    if (dataType == "vec3" || dataType == "float3")
        return 3;

    if (dataType == "vec4" || dataType == "float4" || dataType == "quaternion")
        return 4;

    return 0;
}

ParameterEditResult normalizeNumber (const ParamSpec& param, const std::string& rawValue)
{
    double value = 0.0;
    if (! parseDoubleStrict (rawValue, value))
        return { false, {}, "invalid number for param: " + param.id };

    double minimum = 0.0;
    double maximum = 0.0;
    if (parseRange (param.range, minimum, maximum))
        value = std::clamp (value, minimum, maximum);

    return { true, formatDouble (value), {} };
}

ParameterEditResult normalizeInteger (const ParamSpec& param, const std::string& rawValue)
{
    int value = 0;
    if (! parseIntStrict (rawValue, value))
        return { false, {}, "invalid integer for param: " + param.id };

    double minimum = 0.0;
    double maximum = 0.0;
    if (parseRange (param.range, minimum, maximum))
        value = std::clamp (value, static_cast<int> (minimum), static_cast<int> (maximum));

    return { true, std::to_string (value), {} };
}

ParameterEditResult normalizeBoolean (const ParamSpec& param, const std::string& rawValue)
{
    const auto value = lowercased (trim (rawValue));
    if (value == "true" || value == "1" || value == "on" || value == "yes")
        return { true, "true", {} };

    if (value == "false" || value == "0" || value == "off" || value == "no")
        return { true, "false", {} };

    return { false, {}, "invalid boolean for param: " + param.id };
}

ParameterEditResult normalizeEnum (const ParamSpec& param, const std::string& rawValue)
{
    const auto value = trim (rawValue);
    const auto options = enumOptions (param);
    if (std::find (options.begin(), options.end(), value) == options.end())
        return { false, {}, "invalid enum option for param: " + param.id };

    return { true, value, {} };
}

ParameterEditResult normalizeVector (const ParamSpec& param, const std::string& rawValue, int componentCount)
{
    const auto parts = split (rawValue, ',');
    if (static_cast<int> (parts.size()) != componentCount)
        return { false, {}, "invalid vector component count for param: " + param.id };

    double minimum = 0.0;
    double maximum = 0.0;
    const auto hasRange = parseRange (param.range, minimum, maximum);

    std::ostringstream out;
    for (int index = 0; index < componentCount; ++index)
    {
        double value = 0.0;
        if (! parseDoubleStrict (parts[static_cast<size_t> (index)], value))
            return { false, {}, "invalid vector component for param: " + param.id };

        if (hasRange)
            value = std::clamp (value, minimum, maximum);

        if (index > 0)
            out << ",";

        out << formatDouble (value);
    }

    return { true, out.str(), {} };
}

bool isPathType (const std::string& dataType)
{
    return dataType == "resource.file"
           || dataType == "path"
           || dataType == "path.file"
           || dataType.find ("path.") == 0;
}
}

ParameterControlState parameterControlForParam (const ParamSpec& param)
{
    ParameterControlState control;
    control.dataType = param.dataType;
    control.options = enumOptions (param);
    control.hasRange = parseRange (param.range, control.minimum, control.maximum);
    control.componentCount = vectorComponentCount (param.dataType);

    if (! control.options.empty() || param.dataType == "enum")
        control.kind = ParameterControlKind::enumMenu;
    else if (param.dataType == "float" || param.dataType == "double")
        control.kind = ParameterControlKind::floatSlider;
    else if (param.dataType == "int")
        control.kind = ParameterControlKind::integerStepper;
    else if (param.dataType == "bool")
        control.kind = ParameterControlKind::toggle;
    else if (control.componentCount > 0)
        control.kind = ParameterControlKind::vectorEditor;
    else if (param.dataType.rfind ("text.", 0) == 0)
        control.kind = ParameterControlKind::multilineText;
    else if (isPathType (param.dataType))
        control.kind = ParameterControlKind::pathField;
    else if (param.dataType == "string")
        control.kind = ParameterControlKind::textField;
    else
        control.kind = ParameterControlKind::unsupported;

    return control;
}

ParameterEditResult normalizeParameterEdit (const ParamSpec& param, const std::string& rawValue)
{
    const auto control = parameterControlForParam (param);

    switch (control.kind)
    {
        case ParameterControlKind::floatSlider:    return normalizeNumber (param, rawValue);
        case ParameterControlKind::integerStepper: return normalizeInteger (param, rawValue);
        case ParameterControlKind::toggle:         return normalizeBoolean (param, rawValue);
        case ParameterControlKind::enumMenu:       return normalizeEnum (param, rawValue);
        case ParameterControlKind::vectorEditor:   return normalizeVector (param, rawValue, control.componentCount);
        case ParameterControlKind::textField:
        case ParameterControlKind::multilineText:
        case ParameterControlKind::pathField:      return { true, rawValue, {} };
        case ParameterControlKind::unsupported:    return { false, {}, "unsupported param type: " + param.dataType };
    }

    return { false, {}, "unsupported param type: " + param.dataType };
}

CommandResult setTypedParam (GraphSession& session,
                             const std::string& nodeId,
                             const ParamSpec& param,
                             const std::string& rawValue)
{
    const auto normalized = normalizeParameterEdit (param, rawValue);
    if (! normalized.ok)
        return { false, normalized.error };

    return setParam (session, nodeId, param.id, normalized.value);
}
}
