#include "C2StorageProofRunner.h"

#include "CompoundPatch.h"
#include "GraphEndpoint.h"
#include "InteractionContract.h"
#include "ProofReports.h"
#include "ProofRunSupport.h"
#include "StorageContract.h"

#include <algorithm>

namespace myworld
{
namespace
{
constexpr const char* displayName = "C2 storage";
constexpr const char* directoryName = "c2-storage-proof";
constexpr const char* reportFileName = "reload_report.json";
constexpr const char* savedPatchFileName = "saved_main.patch.json";
constexpr const char* workFixturePath = "fixtures/storage/c2-compound-work/myworld.work.json";
constexpr const char* loudnessCompoundFixturePath = "fixtures/compounds/loudness.compound.json";
constexpr const char* loudnessCompoundNodeId = "library_loud1";

bool hasEdgeId (const GraphContract& graph, const std::string& edgeId)
{
    return std::any_of (graph.editorGraph.edges.begin(),
                        graph.editorGraph.edges.end(),
                        [&edgeId] (const auto& edge) {
                            return edge.id == edgeId;
                        });
}

C2StorageProofRunResult makeInitialResult (const C2StorageProofRunRequest& request)
{
    C2StorageProofRunResult result;
    result.outputDirectory = request.outputDirectory;
    result.reportPath = request.outputDirectory / reportFileName;
    result.artifactPaths = { result.reportPath, request.outputDirectory / savedPatchFileName };
    return result;
}
}

const char* c2StorageProofDisplayName()
{
    return displayName;
}

const char* c2StorageProofDirectoryName()
{
    return directoryName;
}

C2StorageProofRunResult runC2StorageProof (const C2StorageProofRunRequest& request)
{
    auto result = makeInitialResult (request);
    const auto savedPatchFile = request.outputDirectory / savedPatchFileName;

    const auto writeReport = [&] (bool ok,
                                  const std::string& workManifestPath,
                                  const std::string& saveStatus,
                                  const GraphSession& session,
                                  bool publicInputEdge,
                                  bool publicOutputEdge,
                                  bool monoMixLayout,
                                  double monoMixX,
                                  double monoMixY,
                                  const std::string& error)
    {
        return writeProofTextFile (result.reportPath,
                                   makeC2StorageReportJson (ok,
                                                            workManifestPath,
                                                            savedPatchFile.string(),
                                                            saveStatus,
                                                            session,
                                                            publicInputEdge,
                                                            publicOutputEdge,
                                                            monoMixLayout,
                                                            monoMixX,
                                                            monoMixY,
                                                            error));
    };

    const auto fail = [&] (const std::string& message)
    {
        const auto writeError = writeReport (false,
                                             {},
                                             {},
                                             makeGraphSession (GraphContract {}),
                                             false,
                                             false,
                                             false,
                                             0.0,
                                             0.0,
                                             message);
        result.ok = false;
        result.status = "failed";
        result.error = writeError.empty() ? message : writeError;
        return result;
    };

    if (const auto error = createProofDirectoryIfMissing (request.outputDirectory); ! error.empty())
        return fail (error);

    std::string workManifestPath;
    PatchDocumentLoadResult loadedPatch;
    std::string lastError;

    for (const auto& candidate : proofCandidatePaths (request.candidateRoots, workFixturePath))
    {
        const auto loaded = loadMainPatchDocumentForWork (candidate.string());
        if (loaded.ok)
        {
            workManifestPath = candidate.string();
            loadedPatch = loaded;
            break;
        }

        lastError = loaded.error;
    }

    if (! loadedPatch.ok)
        return fail (lastError.empty() ? "could not load C2 work fixture" : lastError);

    auto activeSession = makeGraphSession (loadedPatch.document.graph);
    const auto activeDocument = makePatchDocument (loadedPatch.document.id,
                                                  loadedPatch.document.title,
                                                  activeSession.graph);
    const auto saveResult = savePatchDocument (savedPatchFile.string(), activeDocument);

    if (! saveResult.ok)
    {
        const auto writeError = writeReport (false,
                                             workManifestPath,
                                             saveResult.status,
                                             activeSession,
                                             false,
                                             false,
                                             false,
                                             0.0,
                                             0.0,
                                             saveResult.error);
        result.ok = false;
        result.status = "failed";
        result.error = writeError.empty() ? saveResult.error : writeError;
        return result;
    }

    const auto reloadedPatch = loadPatchDocument (savedPatchFile.string());
    if (! reloadedPatch.ok)
    {
        const auto writeError = writeReport (false,
                                             workManifestPath,
                                             saveResult.status,
                                             activeSession,
                                             false,
                                             false,
                                             false,
                                             0.0,
                                             0.0,
                                             reloadedPatch.error);
        result.ok = false;
        result.status = "failed";
        result.error = writeError.empty() ? reloadedPatch.error : writeError;
        return result;
    }

    auto reloadedSession = makeGraphSession (reloadedPatch.document.graph);

    CompoundPatchLoadResult loadedCompound;
    for (const auto& candidate : proofCandidatePaths (request.candidateRoots, loudnessCompoundFixturePath))
    {
        const auto loaded = loadCompoundPatchSpec (candidate.string());
        if (loaded.ok)
        {
            loadedCompound = loaded;
            break;
        }

        lastError = loaded.error;
    }

    if (! loadedCompound.ok)
    {
        const auto error = lastError.empty() ? "could not load loudness compound fixture" : lastError;
        const auto writeError = writeReport (false,
                                             workManifestPath,
                                             saveResult.status,
                                             reloadedSession,
                                             false,
                                             false,
                                             false,
                                             0.0,
                                             0.0,
                                             error);
        result.ok = false;
        result.status = "failed";
        result.error = writeError.empty() ? error : writeError;
        return result;
    }

    const auto relayoutGraph = makeCompoundPatchInteractionGraph (loadedCompound.spec,
                                                                  loudnessCompoundNodeId,
                                                                  reloadedSession.graph);
    const auto* monoMix = findEditorNode (relayoutGraph, "library_loud1/mono_mix");
    const auto monoMixX = monoMix == nullptr ? 0.0 : monoMix->position.x;
    const auto monoMixY = monoMix == nullptr ? 0.0 : monoMix->position.y;
    const auto publicInputEdge = hasEdgeId (reloadedSession.graph, "edge.live_audio.channels.library_loud1.audio.in");
    const auto publicOutputEdge = hasEdgeId (reloadedSession.graph, "edge.library_loud1.out.midi_loudness.value");
    const auto monoMixLayout = monoMix != nullptr && monoMixX == 358.0 && monoMixY == 146.0;
    const auto graphCountsMatch = reloadedSession.graph.editorGraph.edges.size()
                                  == reloadedSession.graph.runtimeGraph.edges.size();
    const auto ok = publicInputEdge && publicOutputEdge && monoMixLayout && graphCountsMatch;
    const auto error = ok ? std::string {} : "reloaded C2 graph did not match expected compound work";

    if (const auto writeError = writeReport (ok,
                                             workManifestPath,
                                             saveResult.status,
                                             reloadedSession,
                                             publicInputEdge,
                                             publicOutputEdge,
                                             monoMixLayout,
                                             monoMixX,
                                             monoMixY,
                                             error);
        ! writeError.empty())
    {
        result.ok = false;
        result.status = "failed";
        result.error = writeError;
        return result;
    }

    result.ok = ok;
    result.status = ok ? "dumped" : "mismatch";
    result.error = error;
    return result;
}
}
