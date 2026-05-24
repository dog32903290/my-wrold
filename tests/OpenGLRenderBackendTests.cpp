#include "OpenGLRenderBackend.h"

#include <juce_opengl/juce_opengl.h>

#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
void expect (bool condition, const std::string& message)
{
    if (! condition)
    {
        std::cerr << "FAIL: " << message << '\n';
        std::exit (1);
    }
}

void expectEqual (const std::string& actual, const std::string& expected, const std::string& message)
{
    expect (actual == expected, message + " expected " + expected + " got " + actual);
}
}

int main()
{
    juce::OpenGLContext context;
    myworld::OpenGLRenderBackend backend (context);

    expectEqual (backend.backendName(), "OpenGL", "backend name");
    expectEqual (backend.lastStatus(), "waiting for GL context", "initial status");

    std::cout << "opengl render backend ok\n";
    return 0;
}
