#include "JsonWriter.h"

#include <iomanip>
#include <ostream>
#include <sstream>

namespace myworld
{
std::string jsonEscaped (const std::string& text)
{
    std::ostringstream out;

    for (const auto character : text)
    {
        switch (character)
        {
            case '"':  out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
            {
                const auto code = static_cast<unsigned char> (character);
                if (code < 0x20)
                    out << "\\u" << std::hex << std::setw (4) << std::setfill ('0') << static_cast<int> (code);
                else
                    out << character;
                break;
            }
        }
    }

    return out.str();
}

std::string jsonQuoted (const std::string& text)
{
    return "\"" + jsonEscaped (text) + "\"";
}

void appendJsonStringArray (std::ostream& out, const std::vector<std::string>& values)
{
    out << "[";

    for (size_t index = 0; index < values.size(); ++index)
    {
        if (index != 0)
            out << ", ";

        out << jsonQuoted (values[index]);
    }

    out << "]";
}
}
