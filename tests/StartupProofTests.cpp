#include "StartupProof.h"

#include <cstdlib>
#include <iostream>

namespace
{
void expect (bool condition, const char* message)
{
    if (! condition)
    {
        std::cerr << message << '\n';
        std::exit (1);
    }
}
}

int main()
{
    myworld::StartupProofOptions empty;
    expect (! myworld::hasStartupProofRequest (empty), "empty options should not request startup proof");
    expect (myworld::startupProofTasks (empty).empty(), "empty options should not create startup proof tasks");

    myworld::StartupProofOptions selected;
    selected.dumpV1ShaderProof = true;
    selected.dumpA1AudioProof = true;
    selected.dumpLiveIOProof = true;
    selected.dumpC2StorageProof = true;
    selected.dumpAPPWorkbenchSessionProof = true;
    selected.dumpProjectCreationProof = true;
    selected.dumpCreatedProjectOpenProof = true;
    selected.dumpAppSaveProof = true;
    selected.dumpAppStatusProof = true;
    selected.dumpWorkbenchStatusSurfaceProof = true;
    selected.dumpWorkbenchGraphSurfaceProof = true;

    const auto selectedTasks = myworld::startupProofTasks (selected);
    expect (myworld::hasStartupProofRequest (selected), "selected options should request startup proof");
    expect (selectedTasks.size() == 11, "selected options should create eleven tasks");
    expect (selectedTasks[0].id == myworld::StartupProofTaskId::v1Shader, "v1 task should keep first position");
    expect (selectedTasks[0].delayMilliseconds == 750, "v1 task should keep 750ms startup delay");
    expect (selectedTasks[1].id == myworld::StartupProofTaskId::a1Audio, "a1 task should keep second position");
    expect (selectedTasks[1].delayMilliseconds == 2500, "a1 task should keep 2500ms startup delay");
    expect (selectedTasks[2].id == myworld::StartupProofTaskId::liveIO, "live io task should keep third position");
    expect (selectedTasks[2].delayMilliseconds == 500, "live io task should keep 500ms startup delay");
    expect (selectedTasks[3].id == myworld::StartupProofTaskId::c2Storage, "c2 task should keep fourth position");
    expect (selectedTasks[3].delayMilliseconds == 500, "c2 task should keep 500ms startup delay");
    expect (selectedTasks[4].id == myworld::StartupProofTaskId::appWorkbenchSession,
            "app workbench task should be stable app task");
    expect (selectedTasks[4].delayMilliseconds == 500, "app workbench task should keep 500ms startup delay");
    expect (selectedTasks[5].id == myworld::StartupProofTaskId::projectCreation,
            "project creation task should be selected");
    expect (selectedTasks[5].delayMilliseconds == 500, "project creation task should keep 500ms startup delay");
    expect (selectedTasks[6].id == myworld::StartupProofTaskId::createdProjectOpen,
            "created project open task should be selected");
    expect (selectedTasks[6].delayMilliseconds == 500, "created project open task should keep 500ms startup delay");
    expect (selectedTasks[7].id == myworld::StartupProofTaskId::appSave,
            "app save task should be selected");
    expect (selectedTasks[7].delayMilliseconds == 500, "app save task should keep 500ms startup delay");
    expect (selectedTasks[8].id == myworld::StartupProofTaskId::appStatus,
            "app status task should be selected");
    expect (selectedTasks[8].delayMilliseconds == 500, "app status task should keep 500ms startup delay");
    expect (selectedTasks[9].id == myworld::StartupProofTaskId::workbenchStatusSurface,
            "workbench status surface task should be selected");
    expect (selectedTasks[9].delayMilliseconds == 500,
            "workbench status surface task should keep 500ms startup delay");
    expect (selectedTasks[10].id == myworld::StartupProofTaskId::workbenchGraphSurface,
            "workbench graph surface task should be selected");
    expect (selectedTasks[10].delayMilliseconds == 500,
            "workbench graph surface task should keep 500ms startup delay");

    myworld::StartupProofOptions all;
    all.dumpV1ShaderProof = true;
    all.dumpA1AudioProof = true;
    all.dumpLiveIOProof = true;
    all.dumpC2StorageProof = true;
    all.dumpC3SaveWorkProof = true;
    all.dumpC4AIWorkerSaveWorkProof = true;
    all.dumpC5ModulePublishProof = true;
    all.dumpC5AIWorkerModulePublishProof = true;
    all.dumpC5VisibleModulePublishProof = true;
    all.dumpC6AnalyzerFamilyProof = true;
    all.dumpC6AIRepairLoopProof = true;
    all.dumpPVAttackDetectorProof = true;
    all.dumpPVDensityDetectorProof = true;
    all.dumpPVSilenceDetectorProof = true;
    all.dumpPVSustainDetectorProof = true;
    all.dumpPVResidueDetectorProof = true;
    all.dumpPVAggregatePressureProof = true;
    all.dumpPVB1AnalyzerEnvironmentProof = true;
    all.dumpAPP1WorkbenchSessionProof = true;
    all.dumpAPP2WorkbenchOpenStatusProof = true;
    all.dumpAPPWorkbenchSessionProof = true;
    all.dumpProjectCreationProof = true;
    all.dumpCreatedProjectOpenProof = true;
    all.dumpAppSaveProof = true;
    all.dumpAppStatusProof = true;
    all.dumpWorkbenchStatusSurfaceProof = true;
    all.dumpWorkbenchGraphSurfaceProof = true;

    expect (myworld::hasStartupProofRequest (all), "all options should request startup proof");
    expect (myworld::startupProofTasks (all).size() == 27, "all options should create every startup proof task");

    return 0;
}
