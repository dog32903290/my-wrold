#include "CompoundPatch.h"
#include "GraphEndpoint.h"
#include "InteractionContract.h"
#include "StorageCommand.h"
#include "StorageContract.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
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

bool hasEdgeId (const myworld::GraphContract& graph, const std::string& edgeId)
{
    return std::find_if (graph.editorGraph.edges.begin(),
                         graph.editorGraph.edges.end(),
                         [&edgeId] (const auto& edge) {
                             return edge.id == edgeId;
                         }) != graph.editorGraph.edges.end();
}

bool containsCommand (const std::vector<std::string>& commands, const std::string& command)
{
    return std::find (commands.begin(), commands.end(), command) != commands.end();
}

std::string readTextFile (const std::filesystem::path& path)
{
    std::ifstream input (path);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

void copyC2WorkFixture (const std::filesystem::path& workRoot)
{
    std::filesystem::create_directories (workRoot / "patches");
    std::filesystem::copy_file ("fixtures/storage/c2-compound-work/myworld.work.json",
                                workRoot / "myworld.work.json",
                                std::filesystem::copy_options::overwrite_existing);
    std::filesystem::copy_file ("fixtures/storage/c2-compound-work/patches/main.patch.json",
                                workRoot / "patches" / "main.patch.json",
                                std::filesystem::copy_options::overwrite_existing);
}

void expectCommandOk (const std::string& command, const std::string& message)
{
    const auto exitCode = std::system (command.c_str());
    expect (exitCode == 0, message);
}
}

int main()
{
    const auto loadedCompound = myworld::loadCompoundPatchSpec ("fixtures/compounds/loudness.compound.json");
    expect (loadedCompound.ok, loadedCompound.error);

    const auto workRoot = std::filesystem::temp_directory_path() / "my-world-c3-save-work-command";
    std::filesystem::remove_all (workRoot);
    copyC2WorkFixture (workRoot);

    const auto workManifestPath = workRoot / "myworld.work.json";
    const auto loadedMain = myworld::loadMainPatchDocumentForWork (workManifestPath.string());
    expect (loadedMain.ok, loadedMain.error);

    auto session = myworld::makeGraphSession (loadedMain.document.graph);
    expect (myworld::moveNode (session, "library_loud1", 13.0, 7.0).ok,
            "dirty graph session before save_work");
    expect (session.dirty, "session is dirty before save_work");

    const auto result = myworld::saveWork (session, workManifestPath.string());
    expect (result.ok, result.error);
    expect (result.status == "save-ok commit-pending", "save_work status");
    expect (! session.dirty, "save_work clears dirty graph state after PatchDocument write");
    expect (containsCommand (session.commandLog, "save_work:save-ok commit-pending"),
            "save_work command status logged");

    const auto savedPatchPath = workRoot / "patches" / "main.patch.json";
    const auto savedText = readTextFile (savedPatchPath);
    expect (savedText.find ("interaction-state-v1") == std::string::npos,
            "save_work must not use temporary interaction serializer");
    expect (savedText.find ("library_loud1.audio.in") != std::string::npos,
            "saved PatchDocument keeps public input port edge");
    expect (savedText.find ("library_loud1.out") != std::string::npos,
            "saved PatchDocument keeps public output port edge");

    const auto reloaded = myworld::loadPatchDocument (savedPatchPath.string());
    expect (reloaded.ok, reloaded.error);
    expect (hasEdgeId (reloaded.document.graph, "edge.live_audio.channels.library_loud1.audio.in"),
            "save_work reloaded public input edge");
    expect (hasEdgeId (reloaded.document.graph, "edge.library_loud1.out.midi_loudness.value"),
            "save_work reloaded public output edge");

    const auto reloadedExpandedGraph = myworld::makeCompoundPatchInteractionGraph (
        loadedCompound.spec,
        "library_loud1",
        reloaded.document.graph);
    const auto* monoMix = myworld::findEditorNode (reloadedExpandedGraph, "library_loud1/mono_mix");
    expect (monoMix != nullptr, "save_work reloaded expanded mono_mix");
    expect (monoMix->position.x == 358.0, "save_work preserves expanded child layout x");
    expect (monoMix->position.y == 146.0, "save_work preserves expanded child layout y");

    const auto saveLogPath = workRoot / ".myworld" / "save_log.jsonl";
    expect (std::filesystem::exists (saveLogPath), "save_work writes save log");
    const auto saveLog = readTextFile (saveLogPath);
    expect (saveLog.find ("\"command\": \"save_work\"") != std::string::npos,
            "save log records save_work command");
    expect (saveLog.find ("\"status\": \"save-ok commit-pending\"") != std::string::npos,
            "save log records save_work status");

    const auto loadedSaveLog = myworld::loadSaveLog (saveLogPath.string());
    expect (loadedSaveLog.ok, loadedSaveLog.error);
    expect (loadedSaveLog.entries.size() == 1, "save log has one entry");
    expect (loadedSaveLog.entries.front().command == "save_work", "save log entry command reads back");
    expect (loadedSaveLog.entries.front().status == "save-ok commit-pending", "save log entry status reads back");
    expect (loadedSaveLog.entries.front().commitStatus == "not-started", "save log entry commit status reads back");

    std::filesystem::remove_all (workRoot);

    const auto gitWorkRoot = std::filesystem::temp_directory_path() / "my-world-c3-save-work-git";
    std::filesystem::remove_all (gitWorkRoot);
    copyC2WorkFixture (gitWorkRoot);
    expectCommandOk ("git -C " + gitWorkRoot.string() + " init --quiet", "init temp work git repo");
    expectCommandOk ("git -C " + gitWorkRoot.string() + " config user.email c3-save-work@example.local",
                     "set temp git user email");
    expectCommandOk ("git -C " + gitWorkRoot.string() + " config user.name C3SaveWork",
                     "set temp git user name");
    expectCommandOk ("git -C " + gitWorkRoot.string() + " add myworld.work.json patches/main.patch.json",
                     "stage temp initial work");
    expectCommandOk ("git -C " + gitWorkRoot.string() + " commit --quiet -m initial-work",
                     "commit temp initial work");

    const auto gitWorkManifestPath = gitWorkRoot / "myworld.work.json";
    const auto gitLoadedMain = myworld::loadMainPatchDocumentForWork (gitWorkManifestPath.string());
    expect (gitLoadedMain.ok, gitLoadedMain.error);

    auto gitSession = myworld::makeGraphSession (gitLoadedMain.document.graph);
    expect (myworld::moveNode (gitSession, "library_loud1", 21.0, 9.0).ok,
            "dirty git graph session before save_work");

    myworld::SaveWorkOptions saveWithCommit;
    saveWithCommit.startLocalGitCommit = true;
    saveWithCommit.commitMessage = "C3.4 test save_work";

    const auto gitSave = myworld::saveWork (gitSession, gitWorkManifestPath.string(), saveWithCommit);
    expect (gitSave.ok, gitSave.error);
    expect (gitSave.status == "save-ok commit-pending", "git save_work returns commit pending");
    expect (gitSave.commitStatus == "commit-pending", "git save_work commit status pending");
    expect (gitSave.commitJob != nullptr, "git save_work returns background commit job");

    const auto commitResult = gitSave.commitJob->wait();
    expect (commitResult.ok, commitResult.error);
    expect (commitResult.status == "saved-and-committed", "background git commit final status");
    expect (! commitResult.commitId.empty(), "background git commit id");

    const auto gitSaveLog = myworld::loadSaveLog (gitSave.saveLogPath);
    expect (gitSaveLog.ok, gitSaveLog.error);
    expect (gitSaveLog.entries.size() >= 2, "git save log records pending and final entries");
    expect (gitSaveLog.entries.back().status == "saved-and-committed", "git save log final status reads back");
    expect (gitSaveLog.entries.back().commitStatus == "saved-and-committed",
            "git save log final commit status reads back");
    expect (gitSaveLog.entries.back().commitId == commitResult.commitId,
            "git save log final commit id reads back");

    std::filesystem::remove_all (gitWorkRoot);

    std::cout << "save_work command contract ok\n";
    return 0;
}
