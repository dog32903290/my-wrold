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
    expectNear (timeline.bpm, 120.0, "default bpm");
    expectNear (timeline.beatsPerBar, 4.0, "default meter");
    expectNear (timeline.framesPerSecond, 60.0, "default fps");
    expectNear (myworld::barsToSeconds (timeline, 1.0), 2.0, "bars to seconds");
    expectNear (myworld::secondsToBars (timeline, 3.0), 1.5, "seconds to bars");
    expectNear (myworld::barsToFrames (timeline, 1.0), 120.0, "bars to frames");
    expectNear (myworld::framesToBars (timeline, 30.0), 0.25, "frames to bars");

    auto session = myworld::makeGraphSession (myworld::makeDefaultShaderOutputGraph());
    const auto tempo = myworld::setTimelineTempo (session, 90.0);
    expect (tempo.ok, tempo.message);
    expect (session.commandLog.back() == "set_timeline_tempo", "tempo command logged");
    expectNear (session.timeline.bpm, 90.0, "tempo command changes bpm");
    expectNear (myworld::barsToSeconds (session.timeline, 1.0), 2.666666666667, "tempo affects bars");
    expect (myworld::undo (session), "tempo undo");
    expectNear (session.timeline.bpm, 120.0, "undo restores tempo");
    expect (myworld::redo (session), "tempo redo");
    expectNear (session.timeline.bpm, 90.0, "redo reapplies tempo");

    const auto commandCount = session.commandLog.size();
    const auto invalidTempo = myworld::setTimelineTempo (session, 0.0);
    expect (! invalidTempo.ok, "invalid tempo rejected");
    expectNear (session.timeline.bpm, 90.0, "invalid tempo leaves state unchanged");
    expect (session.commandLog.size() == commandCount, "invalid tempo does not log command");

    expect (myworld::setTimelineFramesPerSecond (session, 24.0).ok, "set fps");
    expect (myworld::setTimelinePositionBars (session, 3.5).ok, "set position bars");
    expect (myworld::setTimelineLoop (session, 1.0, 5.0, true).ok, "set loop bars");
    expectNear (session.timeline.framesPerSecond, 24.0, "fps command");
    expectNear (session.timeline.positionBars, 3.5, "position command");
    expectNear (session.timeline.loopStartBars, 1.0, "loop start command");
    expectNear (session.timeline.loopEndBars, 5.0, "loop end command");
    expect (session.timeline.looping, "loop enabled");
    const auto invalidLoop = myworld::setTimelineLoop (session, 8.0, 2.0, true);
    expect (! invalidLoop.ok, "invalid loop rejected");
    expectNear (session.timeline.loopStartBars, 1.0, "invalid loop leaves start unchanged");
    expectNear (session.timeline.loopEndBars, 5.0, "invalid loop leaves end unchanged");

    const auto document = myworld::makePatchDocument ("patch.timeline",
                                                      "Timeline",
                                                      session.graph,
                                                      session.outputView,
                                                      session.timeline);
    const auto json = myworld::toJson (document);
    expectContains (json, "\"timeline\"", "patch json");
    expectContains (json, "\"bpm\": 90", "patch json");
    expectContains (json, "\"framesPerSecond\": 24", "patch json");
    expectContains (json, "\"positionBars\": 3.5", "patch json");

    const auto parsed = myworld::parsePatchDocument (json);
    expect (parsed.ok, parsed.error);
    expectNear (parsed.document.timeline.bpm, 90.0, "patch reloads bpm");
    expectNear (parsed.document.timeline.framesPerSecond, 24.0, "patch reloads fps");
    expectNear (parsed.document.timeline.positionBars, 3.5, "patch reloads position");
    expect (parsed.document.timeline.looping, "patch reloads loop enabled");

    const auto root = std::filesystem::temp_directory_path() / "my-world-timeline-state-tests";
    std::filesystem::remove_all (root);
    const auto manifestPath = root / "myworld.work.json";
    const auto patchPath = root / "patches" / "main.patch.json";

    writeText (manifestPath, myworld::toJson (myworld::makeMinimalWorkProject ("work.timeline", "Timeline Work")));
    const auto initialSave = myworld::savePatchDocument (
        patchPath.string(),
        myworld::makePatchDocument ("patch.timeline-main",
                                    "Timeline Main",
                                    myworld::makeDefaultShaderOutputGraph()));
    expect (initialSave.ok, initialSave.error);

    const auto loadedMain = myworld::loadMainPatchDocumentForWork (manifestPath.string());
    expect (loadedMain.ok, loadedMain.error);
    auto saveSession = myworld::makeGraphSession (loadedMain.document.graph);
    saveSession.outputView = loadedMain.document.outputView;
    saveSession.timeline = loadedMain.document.timeline;
    expect (myworld::setTimelineTempo (saveSession, 72.0).ok, "save session tempo");
    expect (myworld::setTimelineFramesPerSecond (saveSession, 30.0).ok, "save session fps");
    expect (myworld::setTimelinePositionBars (saveSession, 6.25).ok, "save session position");

    const auto saveResult = myworld::saveWork (saveSession, manifestPath.string());
    expect (saveResult.ok, saveResult.error);

    const auto reloadedMain = myworld::loadMainPatchDocumentForWork (manifestPath.string());
    expect (reloadedMain.ok, reloadedMain.error);
    expectNear (reloadedMain.document.timeline.bpm, 72.0, "save_work preserves bpm");
    expectNear (reloadedMain.document.timeline.framesPerSecond, 30.0, "save_work preserves fps");
    expectNear (reloadedMain.document.timeline.positionBars, 6.25, "save_work preserves position");

    std::filesystem::remove_all (root);

    std::cout << "timeline state ok\n";
    return 0;
}
