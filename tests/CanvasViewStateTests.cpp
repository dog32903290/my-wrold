#include "InteractionContract.h"

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

void expectNear (double actual, double expected, double tolerance, const std::string& message)
{
    expect (std::abs (actual - expected) <= tolerance, message);
}
}

int main()
{
    myworld::CanvasViewState view;
    view.scale = 2.0;
    view.scrollX = 10.0;
    view.scrollY = -4.0;

    const myworld::CanvasPoint canvasPoint { 42.0, 18.0 };
    const auto screenPoint = myworld::canvasToScreen (view, canvasPoint);
    const auto roundtrip = myworld::screenToCanvas (view, screenPoint);

    expectNear (roundtrip.x, canvasPoint.x, 0.0001, "canvas x roundtrip");
    expectNear (roundtrip.y, canvasPoint.y, 0.0001, "canvas y roundtrip");

    const auto panned = myworld::panView (view, 12.0, -8.0);
    expectNear (panned.scrollX, 22.0, 0.0001, "pan x");
    expectNear (panned.scrollY, -12.0, 0.0001, "pan y");

    const myworld::ScreenPoint focus { 320.0, 240.0 };
    const auto beforeFocusCanvas = myworld::screenToCanvas (view, focus);
    const auto zoomed = myworld::zoomViewAround (view, 1.5, focus);
    const auto afterFocusCanvas = myworld::screenToCanvas (zoomed, focus);

    expectNear (zoomed.scale, 3.0, 0.0001, "zoom scale");
    expectNear (afterFocusCanvas.x, beforeFocusCanvas.x, 0.0001, "zoom preserves focus x");
    expectNear (afterFocusCanvas.y, beforeFocusCanvas.y, 0.0001, "zoom preserves focus y");

    const auto rejected = myworld::zoomViewAround (view, -1.0, focus);
    expectNear (rejected.scale, view.scale, 0.0001, "invalid zoom rejected");

    std::cout << "canvas view state ok\n";
    return 0;
}
