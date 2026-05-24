#include "OpenGLRenderBackend.h"

#include <vector>

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

OpenGLRenderBackend::OpenGLRenderBackend (juce::OpenGLContext& context)
    : openGLContext (context)
{
}

OpenGLRenderBackend::~OpenGLRenderBackend()
{
    release();
}

const char* OpenGLRenderBackend::backendName() const
{
    return "OpenGL";
}

std::string OpenGLRenderBackend::lastStatus() const
{
    return status;
}

void OpenGLRenderBackend::initialise()
{
    createFullScreenQuad();
    status = "waiting for shader";
}

ShaderCompileResult OpenGLRenderBackend::compileShader (const std::string& fragmentSource)
{
    auto nextProgram = std::make_unique<juce::OpenGLShaderProgram> (openGLContext);

    if (nextProgram->addVertexShader (juce::OpenGLHelpers::translateVertexShaderToV3 (vertexShaderSource()))
        && nextProgram->addFragmentShader (juce::OpenGLHelpers::translateFragmentShaderToV3 (fragmentSource))
        && nextProgram->link())
    {
        shaderProgram = std::move (nextProgram);
        shaderProgram->use();

        positionAttribute = makeAttribute (*shaderProgram, "position");
        timeUniform = makeUniform (*shaderProgram, "u_time");
        resolutionUniform = makeUniform (*shaderProgram, "u_resolution");
        frameUniform = makeUniform (*shaderProgram, "u_frame");
        loudnessUniform = makeUniform (*shaderProgram, "u_loudness");

        status = "compiled: GLSL v" + juce::String (juce::OpenGLShaderProgram::getLanguageVersion(), 2).toStdString();
        return { true, status };
    }

    status = "compile failed; keeping last valid frame\n" + nextProgram->getLastError().toStdString();
    return { false, status };
}

void OpenGLRenderBackend::resize (int width, int height, float scale)
{
    viewportWidth = juce::jmax (1, width);
    viewportHeight = juce::jmax (1, height);
    viewportScale = scale;
}

void OpenGLRenderBackend::renderFrame (const RenderFrameInput& input)
{
    using namespace ::juce::gl;

    glViewport (0, 0, viewportWidth, viewportHeight);
    juce::OpenGLHelpers::clear (juce::Colour::fromRGB (8, 9, 12));

    if (shaderProgram == nullptr)
        return;

    shaderProgram->use();

    if (timeUniform != nullptr)
        timeUniform->set (static_cast<float> (input.timeSeconds));

    if (resolutionUniform != nullptr)
        resolutionUniform->set (static_cast<float> (viewportWidth), static_cast<float> (viewportHeight));

    if (frameUniform != nullptr)
        frameUniform->set (static_cast<float> (input.frameIndex));

    if (loudnessUniform != nullptr)
        loudnessUniform->set (input.loudness);

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
}

CapturedFrame OpenGLRenderBackend::captureFrame() const
{
    using namespace ::juce::gl;

    CapturedFrame frame;
    frame.width = viewportWidth;
    frame.height = viewportHeight;
    frame.rgba.resize (static_cast<size_t> (viewportWidth) * static_cast<size_t> (viewportHeight) * 4u);

    glPixelStorei (GL_PACK_ALIGNMENT, 1);
    glReadPixels (0, 0, viewportWidth, viewportHeight, GL_RGBA, GL_UNSIGNED_BYTE, frame.rgba.data());

    return frame;
}

void OpenGLRenderBackend::release()
{
    using namespace ::juce::gl;

    positionAttribute.reset();
    timeUniform.reset();
    resolutionUniform.reset();
    frameUniform.reset();
    loudnessUniform.reset();
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

    status = "released";
}

void OpenGLRenderBackend::createFullScreenQuad()
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
}

juce::String OpenGLRenderBackend::vertexShaderSource()
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
