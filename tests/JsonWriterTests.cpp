#include "JsonWriter.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
void expectEqual (const std::string& actual, const std::string& expected, const std::string& message)
{
    if (actual == expected)
        return;

    std::cerr << "FAIL: " << message << "\nexpected: " << expected << "\nactual:   " << actual << '\n';
    std::exit (1);
}
}

int main()
{
    expectEqual (myworld::jsonEscaped ("quote \" slash \\"),
                 "quote \\\" slash \\\\",
                 "escape quote and backslash");
    expectEqual (myworld::jsonEscaped ("line\nrow\tcarriage\r"),
                 "line\\nrow\\tcarriage\\r",
                 "escape common control characters");
    expectEqual (myworld::jsonEscaped (std::string ("prefix ") + static_cast<char> (0x01) + " suffix"),
                 "prefix \\u0001 suffix",
                 "escape unnamed control character");
    expectEqual (myworld::jsonQuoted ("a\"b\\c\n"),
                 "\"a\\\"b\\\\c\\n\"",
                 "quote wraps escaped text");

    std::ostringstream values;
    myworld::appendJsonStringArray (values, { "alpha", "b\"eta", "gamma\n" });
    expectEqual (values.str(),
                 "[\"alpha\", \"b\\\"eta\", \"gamma\\n\"]",
                 "string array uses shared quoting");

    std::ostringstream empty;
    myworld::appendJsonStringArray (empty, {});
    expectEqual (empty.str(), "[]", "empty string array");

    std::cout << "json writer tests ok\n";
    return 0;
}
