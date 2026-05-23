#include "OpenGLShaderPreview.h"

#include "CompoundModule.h"
#include "RuntimeRegistry.h"

#include <atomic>
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

bool writeTextFile (const juce::File& file, const std::string& text)
{
    return file.replaceWithText (juce::String::fromUTF8 (text.data(), static_cast<int> (text.size())),
                                 false,
                                 false,
                                 "\n");
}

bool writePngFile (const juce::File& file, const juce::Image& image)
{
    if (file.existsAsFile() && ! file.deleteFile())
        return false;

    auto output = file.createOutputStream();

    if (output == nullptr)
        return false;

    juce::PNGImageFormat pngFormat;
    return pngFormat.writeImageToStream (image, *output);
}

int mouseButtonIndex (const juce::MouseEvent& event)
{
    if (event.mods.isRightButtonDown())
        return 1;

    if (event.mods.isMiddleButtonDown())
        return 2;

    return 0;
}

juce::File parentDirectory (juce::File file, const int levels)
{
    for (int i = 0; i < levels; ++i)
        file = file.getParentDirectory();

    return file;
}

std::vector<std::string> moduleLibraryCandidatePaths()
{
    const auto libraryPath = juce::String ("fixtures/module-libraries/default.module-library.json");
    const auto executableDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
    const auto buildAppRepoRoot = parentDirectory (executableDir, 5);

    return {
        juce::File::getCurrentWorkingDirectory().getChildFile (libraryPath).getFullPathName().toStdString(),
        buildAppRepoRoot.getChildFile (libraryPath).getFullPathName().toStdString()
    };
}

std::vector<NodeSpec> loadVisibleNodeSpecs()
{
    const auto seedSpecs = makeSeedNodeSpecs();

    for (const auto& path : moduleLibraryCandidatePaths())
    {
        const auto modules = loadCompoundModuleNodeSpecsFromLibrary (path);
        if (modules.ok)
            return mergeNodeSpecs (seedSpecs, modules.specs);
    }

    return seedSpecs;
}

RuntimeRegistry loadVisibleRuntimeRegistry()
{
    for (const auto& path : moduleLibraryCandidatePaths())
    {
        const auto registry = loadRuntimeRegistryFromModuleLibrary (path);
        if (registry.ok)
            return registry.registry;
    }

    return {};
}
}

OpenGLShaderPreview::OpenGLShaderPreview()
    : pendingFragmentShader (defaultFragmentShader())
{
    setWantsKeyboardFocus (true);

    imguiOverlay.onShaderSourceSubmitted = [this] (const std::string& source)
    {
        setFragmentShader (source);
    };

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

void OpenGLShaderPreview::setLoudness (float newLoudness)
{
    loudness.store (juce::jlimit (0.0f, 1.0f, newLoudness), std::memory_order_relaxed);
}

void OpenGLShaderPreview::requestProofDump (juce::File outputDirectory, GraphContract graph)
{
    {
        const juce::ScopedLock lock (proofDumpLock);
        pendingProofDump = std::make_unique<PendingProofDump> (PendingProofDump { std::move (outputDirectory),
                                                                                  std::move (graph) });
    }

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
    lastFrameSeconds = startTimeSeconds;
    frameIndex = 0;
    seedNodeSpecs = loadVisibleNodeSpecs();
    loudnessCompound = makeLoudnessCompoundPatchSpec();
    imguiOverlay.setShaderSource (pendingFragmentShader);
    imguiOverlay.initialise();

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

    const auto nowSeconds = juce::Time::getMillisecondCounterHiRes() * 0.001;
    const auto elapsed = static_cast<float> (nowSeconds - startTimeSeconds);

    if (shaderProgram != nullptr)
    {
        shaderProgram->use();

        if (timeUniform != nullptr)
            timeUniform->set (elapsed);

        if (resolutionUniform != nullptr)
            resolutionUniform->set (static_cast<float> (juce::jmax (1, width)),
                                    static_cast<float> (juce::jmax (1, height)));

        if (frameUniform != nullptr)
            frameUniform->set (static_cast<float> (frameIndex));

        if (loudnessUniform != nullptr)
            loudnessUniform->set (loudness.load (std::memory_order_relaxed));

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

    const auto deltaSeconds = static_cast<float> (nowSeconds - lastFrameSeconds);
    lastFrameSeconds = nowSeconds;
    imguiOverlay.beginFrame (juce::jmax (1, width), juce::jmax (1, height), scale, deltaSeconds);
    imguiOverlay.drawSmokePanel (seedNodeSpecs, loudnessCompound, lastStatus.toStdString());
    imguiOverlay.render();

    handlePendingProofDump (juce::jmax (1, width), juce::jmax (1, height), elapsed, frameIndex);
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

void OpenGLShaderPreview::mouseMove (const juce::MouseEvent& event)
{
    updateImGuiMousePosition (event);
}

void OpenGLShaderPreview::mouseDown (const juce::MouseEvent& event)
{
    grabKeyboardFocus();
    updateImGuiMousePosition (event);
    imguiOverlay.setMouseButton (mouseButtonIndex (event), true);
}

void OpenGLShaderPreview::mouseDrag (const juce::MouseEvent& event)
{
    updateImGuiMousePosition (event);
}

void OpenGLShaderPreview::mouseUp (const juce::MouseEvent& event)
{
    updateImGuiMousePosition (event);
    imguiOverlay.setMouseButton (0, false);
    imguiOverlay.setMouseButton (1, false);
    imguiOverlay.setMouseButton (2, false);
}

void OpenGLShaderPreview::mouseWheelMove (const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    updateImGuiMousePosition (event);
    imguiOverlay.addMouseWheel (wheel.deltaY);
}

bool OpenGLShaderPreview::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey)
    {
        imguiOverlay.requestDeleteSelection();
        openGLContext.triggerRepaint();
        return true;
    }

    return false;
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
        loudnessUniform = makeUniform (*shaderProgram, "u_loudness");

        reportStatus ("compiled: GLSL v" + juce::String (juce::OpenGLShaderProgram::getLanguageVersion(), 2));
        return;
    }

    reportStatus ("compile failed; keeping last valid frame\n" + nextProgram->getLastError());
}

void OpenGLShaderPreview::handlePendingProofDump (int width,
                                                  int height,
                                                  double timeSeconds,
                                                  juce::uint32 currentFrameIndex)
{
    std::unique_ptr<PendingProofDump> dump;

    {
        const juce::ScopedLock lock (proofDumpLock);
        dump = std::move (pendingProofDump);
        pendingProofDump.reset();
    }

    if (dump == nullptr)
        return;

    if (! dump->outputDirectory.createDirectory())
    {
        reportStatus ("proof dump failed: could not create " + dump->outputDirectory.getFullPathName());
        return;
    }

    const auto frameImage = readCurrentFrameBuffer (width, height);
    const auto frameFile = dump->outputDirectory.getChildFile ("frame.png");
    const auto cookOrderFile = dump->outputDirectory.getChildFile ("cook_order.json");
    const auto nodeStatsFile = dump->outputDirectory.getChildFile ("node_stats.json");
    const auto loudnessCompoundFile = dump->outputDirectory.getChildFile ("loudness_compound.json");
    const auto runtimeRegistryFile = dump->outputDirectory.getChildFile ("runtime_registry.json");

    const auto cookOrderWritten = writeTextFile (cookOrderFile, makeCookOrderJson (dump->graph));
    const auto nodeStatsWritten = writeTextFile (nodeStatsFile,
                                                 makeNodeStatsJson (dump->graph,
                                                                    width,
                                                                    height,
                                                                    currentFrameIndex,
                                                                    timeSeconds,
                                                                    "OpenGL",
                                                                    lastStatus.toStdString()));
    const auto loudnessCompoundWritten = writeTextFile (loudnessCompoundFile,
                                                        makeCompoundPatchJson (loudnessCompound));
    const auto runtimeRegistryWritten = writeTextFile (runtimeRegistryFile,
                                                       makeRuntimeRegistryJson (loadVisibleRuntimeRegistry()));
    const auto frameWritten = writePngFile (frameFile, frameImage);

    if (cookOrderWritten && nodeStatsWritten && loudnessCompoundWritten && runtimeRegistryWritten && frameWritten)
    {
        reportStatus ("proof dumped: " + dump->outputDirectory.getFullPathName());
        return;
    }

    reportStatus ("proof dump failed: "
                  + juce::String (cookOrderWritten ? "" : "cook_order.json ")
                  + juce::String (nodeStatsWritten ? "" : "node_stats.json ")
                  + juce::String (loudnessCompoundWritten ? "" : "loudness_compound.json ")
                  + juce::String (runtimeRegistryWritten ? "" : "runtime_registry.json ")
                  + juce::String (frameWritten ? "" : "frame.png"));
}

juce::Image OpenGLShaderPreview::readCurrentFrameBuffer (int width, int height) const
{
    using namespace ::juce::gl;

    std::vector<unsigned char> pixels (static_cast<size_t> (width) * static_cast<size_t> (height) * 4u);

    glPixelStorei (GL_PACK_ALIGNMENT, 1);
    glReadPixels (0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    juce::Image image (juce::Image::ARGB, width, height, true);
    juce::Image::BitmapData bitmap (image, juce::Image::BitmapData::writeOnly);

    for (int y = 0; y < height; ++y)
    {
        const auto sourceY = height - 1 - y;

        for (int x = 0; x < width; ++x)
        {
            const auto sourceIndex = (static_cast<size_t> (sourceY) * static_cast<size_t> (width)
                                      + static_cast<size_t> (x)) * 4u;

            bitmap.setPixelColour (x,
                                   y,
                                   juce::Colour::fromRGBA (pixels[sourceIndex],
                                                           pixels[sourceIndex + 1],
                                                           pixels[sourceIndex + 2],
                                                           pixels[sourceIndex + 3]));
        }
    }

    return image;
}

void OpenGLShaderPreview::releaseGLObjects()
{
    using namespace ::juce::gl;

    imguiOverlay.shutdown();

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
}

void OpenGLShaderPreview::reportStatus (juce::String message)
{
    lastStatus = message;

    if (onStatusMessage != nullptr)
        onStatusMessage (std::move (message));
}

void OpenGLShaderPreview::updateImGuiMousePosition (const juce::MouseEvent& event)
{
    imguiOverlay.setMousePosition (event.position.x, event.position.y);
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
