#include "OpenGLShaderPreview.h"

namespace myworld
{
namespace
{
std::unique_ptr<juce::OpenGLShaderProgram::Uniform> makeUniform (juce::OpenGLShaderProgram& program,
                                                                 const char* name)
{
    using namespace ::juce::gl;

    if (glGetUniformLocation (program.getProgramID(), name) < 0)
        return {};

    return std::make_unique<juce::OpenGLShaderProgram::Uniform> (program, name);
}

std::unique_ptr<juce::OpenGLShaderProgram::Attribute> makeAttribute (juce::OpenGLShaderProgram& program,
                                                                     const char* name)
{
    using namespace ::juce::gl;

    if (glGetAttribLocation (program.getProgramID(), name) < 0)
        return {};

    return std::make_unique<juce::OpenGLShaderProgram::Attribute> (program, name);
}
}

OpenGLShaderPreview::OpenGLShaderPreview()
    : pendingFragmentShader (defaultFragmentShader())
{
    openGLContext.setOpenGLVersionRequired (juce::OpenGLContext::openGL3_2);
    openGLContext.setRenderer (this);
    openGLContext.setContinuousRepainting (false);
    openGLContext.attachTo (*this);

    startTimerHz (60);
}

OpenGLShaderPreview::~OpenGLShaderPreview()
{
    stopTimer();
    onStatusMessage = nullptr;
    openGLContext.detach();
}

void OpenGLShaderPreview::setFragmentShader (std::string source)
{
    const juce::ScopedLock lock (shaderLock);
    pendingFragmentShader = std::move (source);
    compileRequested = true;
    openGLContext.triggerRepaint();
}

void OpenGLShaderPreview::newOpenGLContextCreated()
{
    using namespace ::juce::gl;

    const GLfloat vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f
    };

    glGenVertexArrays (1, &vertexArray);
    glBindVertexArray (vertexArray);

    glGenBuffers (1, &vertexBuffer);
    glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData (GL_ARRAY_BUFFER, static_cast<GLsizeiptr> (sizeof (vertices)), vertices, GL_STATIC_DRAW);
    glBindBuffer (GL_ARRAY_BUFFER, 0);
    glBindVertexArray (0);

    startTimeSeconds = juce::Time::getMillisecondCounterHiRes() * 0.001;
    frameIndex = 0;

    compilePendingShader();
}

void OpenGLShaderPreview::renderOpenGL()
{
    using namespace ::juce::gl;

    jassert (juce::OpenGLHelpers::isContextActive());

    compilePendingShader();

    const auto scale = static_cast<float> (openGLContext.getRenderingScale());
    const auto width = juce::roundToInt (static_cast<float> (getWidth()) * scale);
    const auto height = juce::roundToInt (static_cast<float> (getHeight()) * scale);

    glViewport (0, 0, juce::jmax (1, width), juce::jmax (1, height));
    juce::OpenGLHelpers::clear (juce::Colour::fromRGB (8, 9, 12));

    if (shaderProgram == nullptr)
        return;

    shaderProgram->use();

    const auto nowSeconds = juce::Time::getMillisecondCounterHiRes() * 0.001;
    const auto elapsed = static_cast<float> (nowSeconds - startTimeSeconds);

    if (timeUniform != nullptr)
        timeUniform->set (elapsed);

    if (resolutionUniform != nullptr)
        resolutionUniform->set (static_cast<float> (juce::jmax (1, width)),
                                static_cast<float> (juce::jmax (1, height)));

    if (frameUniform != nullptr)
        frameUniform->set (static_cast<float> (frameIndex));

    glBindVertexArray (vertexArray);
    glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);

    if (positionAttribute != nullptr)
    {
        glVertexAttribPointer (positionAttribute->attributeID, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), nullptr);
        glEnableVertexAttribArray (positionAttribute->attributeID);
    }

    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

    if (positionAttribute != nullptr)
        glDisableVertexAttribArray (positionAttribute->attributeID);

    glBindBuffer (GL_ARRAY_BUFFER, 0);
    glBindVertexArray (0);

    ++frameIndex;
}

void OpenGLShaderPreview::openGLContextClosing()
{
    releaseGLObjects();
}

void OpenGLShaderPreview::timerCallback()
{
    openGLContext.triggerRepaint();
}

void OpenGLShaderPreview::compilePendingShader()
{
    std::string source;

    {
        const juce::ScopedLock lock (shaderLock);

        if (! compileRequested)
            return;

        source = pendingFragmentShader;
        compileRequested = false;
    }

    auto nextProgram = std::make_unique<juce::OpenGLShaderProgram> (openGLContext);

    if (nextProgram->addVertexShader (juce::OpenGLHelpers::translateVertexShaderToV3 (vertexShaderSource()))
        && nextProgram->addFragmentShader (juce::OpenGLHelpers::translateFragmentShaderToV3 (source))
        && nextProgram->link())
    {
        shaderProgram = std::move (nextProgram);
        shaderProgram->use();

        positionAttribute = makeAttribute (*shaderProgram, "position");
        timeUniform = makeUniform (*shaderProgram, "u_time");
        resolutionUniform = makeUniform (*shaderProgram, "u_resolution");
        frameUniform = makeUniform (*shaderProgram, "u_frame");

        reportStatus ("compiled: GLSL v" + juce::String (juce::OpenGLShaderProgram::getLanguageVersion(), 2));
        return;
    }

    reportStatus ("compile failed; keeping last valid frame\n" + nextProgram->getLastError());
}

void OpenGLShaderPreview::releaseGLObjects()
{
    using namespace ::juce::gl;

    positionAttribute.reset();
    timeUniform.reset();
    resolutionUniform.reset();
    frameUniform.reset();
    shaderProgram.reset();

    if (vertexBuffer != 0)
    {
        glDeleteBuffers (1, &vertexBuffer);
        vertexBuffer = 0;
    }

    if (vertexArray != 0)
    {
        glDeleteVertexArrays (1, &vertexArray);
        vertexArray = 0;
    }
}

void OpenGLShaderPreview::reportStatus (juce::String message)
{
    if (onStatusMessage != nullptr)
        onStatusMessage (std::move (message));
}

juce::String OpenGLShaderPreview::vertexShaderSource()
{
    return R"(attribute vec2 position;
varying vec2 v_uv;

void main()
{
    v_uv = position * 0.5 + 0.5;
    gl_Position = vec4(position, 0.0, 1.0);
}
)";
}
}
