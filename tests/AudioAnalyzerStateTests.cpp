#include "AudioAnalyzerState.h"

#include <cmath>
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

void expectNear (float actual, float expected, float tolerance, const std::string& message)
{
    expect (std::abs (actual - expected) <= tolerance, message);
}
}

int main()
{
    myworld::AudioAnalyzerState analyzer;

    const float channel0[] = { 0.0f, 0.5f, -0.5f, 1.0f };
    const float* channels[] = { channel0 };
    analyzer.processBlock (channels, 1, 4, 1.0f);

    auto snapshot = analyzer.getSnapshot();
    expectNear (snapshot.rms, std::sqrt ((0.0f + 0.25f + 0.25f + 1.0f) / 4.0f), 0.0001f, "rms");
    expectNear (snapshot.peak, 1.0f, 0.0001f, "peak");
    expectNear (snapshot.loudness, snapshot.rms, 0.0001f, "loudness follows rms for first proof");
    expect (snapshot.active, "non-silent input should be active");
    expect (snapshot.sampleCounter == 4, "sample counter");

    const float silent[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    const float* silentChannels[] = { silent };
    analyzer.processBlock (silentChannels, 1, 4, 1.0f);

    snapshot = analyzer.getSnapshot();
    expectNear (snapshot.rms, 0.0f, 0.0001f, "silence rms");
    expectNear (snapshot.peak, 0.0f, 0.0001f, "silence peak");
    expect (! snapshot.active, "silence inactive");
    expect (snapshot.sampleCounter == 8, "sample counter accumulates");

    const float left[] = { 1.0f, 1.0f };
    const float right[] = { -1.0f, -1.0f };
    const float* stereo[] = { left, right };
    analyzer.processBlock (stereo, 2, 2, 1.0f);

    snapshot = analyzer.getSnapshot();
    expectNear (snapshot.rms, 0.0f, 0.0001f, "mono mix can cancel opposite phase stereo");
    expectNear (snapshot.peak, 0.0f, 0.0001f, "mono mix peak");

    std::cout << "audio analyzer state ok\n";
    return 0;
}
