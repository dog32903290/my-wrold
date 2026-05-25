#include "InteractionContract.h"
#include "StorageCommand.h"
#include "StorageContract.h"
#include "TimelineState.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
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

void expectNear (double actual, double expected, const std::string& message)
{
    expect (std::abs (actual - expected) < 0.000001,
            message + " expected " + std::to_string (expected) + " got " + std::to_string (actual));
}

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}

void writeText (const std::filesystem::path& path, const std::string& text)
{
    std::filesystem::create_directories (path.parent_path());
    std::ofstream output (path, std::ios::trunc);
    expect (static_cast<bool> (output), "open " + path.string());
    output << text;
    expect (static_cast<bool> (output), "write " + path.string());
}
}

int main()
{
    myworld::TimelineState timeline;
    expect (timeline.transportState == myworld::TimelineTransportState::stopped, "default transport stopped");
    expectNear (timeline.playbackRate, 1.0, "default playback rate");
    expect (timeline.playbackDirection == 1, "default playback direction");

    auto session = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    expect (myworld::playTimeline (session, 1, 1.5).ok, "play forward");
    expect (session.commandLog.back() == "transport_play", "play command logged");
    expect (session.timeline.transportState == myworld::TimelineTransportState::playing, "play sets playing");
    expectNear (session.timeline.playbackRate, 1.5, "play stores rate");
    expect (session.timeline.playbackDirection == 1, "play stores forward direction");
    expect (myworld::undo (session), "play undo");
    expect (session.timeline.transportState == myworld::TimelineTransportState::stopped, "undo restores stopped");
    expect (myworld::redo (session), "play redo");
    expect (session.timeline.transportState == myworld::TimelineTransportState::playing, "redo restores playing");

    const auto commandCount = session.commandLog.size();
    expect (! myworld::playTimeline (session, 1, 0.0).ok, "invalid playback rate rejected");
    expect (session.commandLog.size() == commandCount, "invalid play does not log command");
    expectNear (session.timeline.playbackRate, 1.5, "invalid play leaves rate unchanged");

    expect (myworld::pauseTimeline (session).ok, "pause");
    expect (session.commandLog.back() == "transport_pause", "pause command logged");
    expect (session.timeline.transportState == myworld::TimelineTransportState::paused, "pause sets paused");

    expect (myworld::setTimelineFramesPerSecond (session, 24.0).ok, "set fps");
    expect (myworld::setTimelinePositionBars (session, 2.0).ok, "set position");
    expect (myworld::stepTimelineFrames (session, 48.0).ok, "step forward by frames");
    expect (session.commandLog.back() == "transport_step", "step command logged");
    expectNear (session.timeline.positionBars, 3.0, "48 frames at 24fps and 120bpm is one bar");
    expect (myworld::stepTimelineFrames (session, -24.0).ok, "step backward by frames");
    expectNear (session.timeline.positionBars, 2.5, "negative frame step moves backward");

    expect (myworld::setTimelineLoop (session, 1.0, 4.0, true).ok, "enable loop");
    expect (myworld::playTimeline (session, 1, 1.0).ok, "play before tick");
    expect (myworld::setTimelinePositionBars (session, 3.5).ok, "set looped position");
    const auto tick = myworld::advanceTimelinePlayback (session.timeline, 2.0);
    expect (tick.ok, tick.message);
    expectNear (session.timeline.positionBars, 1.5, "playback tick wraps inside loop range");

    expect (myworld::stopTimeline (session).ok, "stop");
    expect (session.commandLog.back() == "transport_stop", "stop command logged");
    expect (session.timeline.transportState == myworld::TimelineTransportState::stopped, "stop sets stopped");
    expectNear (session.timeline.positionBars, 0.0, "stop resets position to start");

    const auto document = myworld::makePatchDocument ("patch.transport",
                                                      "Transport",
                                                      session.graph,
                                                      session.outputView,
                                                      session.timeline);
    const auto json = myworld::toJson (document);
    expectContains (json, "\"transportState\": \"stopped\"", "patch json");
    expectContains (json, "\"playbackRate\": 1", "patch json");
    expectContains (json, "\"playbackDirection\": 1", "patch json");

    const auto parsed = myworld::parsePatchDocument (json);
    expect (parsed.ok, parsed.error);
    expect (parsed.document.timeline.transportState == myworld::TimelineTransportState::stopped,
            "patch reloads transport state");
    expectNear (parsed.document.timeline.playbackRate, 1.0, "patch reloads playback rate");
    expect (parsed.document.timeline.playbackDirection == 1, "patch reloads playback direction");

    const auto root = std::filesystem::temp_directory_path() / "my-world-transport-control-tests";
    std::filesystem::remove_all (root);
    const auto manifestPath = root / "myworld.work.json";
    const auto patchPath = root / "patches" / "main.patch.json";

    writeText (manifestPath, myworld::toJson (myworld::makeMinimalWorkProject ("work.transport", "Transport Work")));
    const auto initialSave = myworld::savePatchDocument (
        patchPath.string(),
        myworld::makePatchDocument ("patch.transport-main",
                                    "Transport Main",
                                    myworld::makeDefaultShaderOutputGraph()));
    expect (initialSave.ok, initialSave.error);

    const auto loadedMain = myworld::loadMainPatchDocumentForWork (manifestPath.string());
    expect (loadedMain.ok, loadedMain.error);
    auto saveSession = myworld::makeGraphSession (loadedMain.document.graph);
    saveSession.outputView = loadedMain.document.outputView;
    saveSession.timeline = loadedMain.document.timeline;
    expect (myworld::playTimeline (saveSession, -1, 0.5).ok, "save session reverse play");

    const auto saveResult = myworld::saveWork (saveSession, manifestPath.string());
    expect (saveResult.ok, saveResult.error);

    const auto reloadedMain = myworld::loadMainPatchDocumentForWork (manifestPath.string());
    expect (reloadedMain.ok, reloadedMain.error);
    expect (reloadedMain.document.timeline.transportState == myworld::TimelineTransportState::playing,
            "save_work preserves playing state");
    expectNear (reloadedMain.document.timeline.playbackRate, 0.5, "save_work preserves rate");
    expect (reloadedMain.document.timeline.playbackDirection == -1, "save_work preserves direction");

    std::filesystem::remove_all (root);

    std::cout << "transport controls ok\n";
    return 0;
}
