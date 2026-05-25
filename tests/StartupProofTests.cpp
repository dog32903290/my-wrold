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

    const auto selectedTasks = myworld::startupProofTasks (selected);
    expect (myworld::hasStartupProofRequest (selected), "selected options should request startup proof");
    expect (selectedTasks.size() == 5, "selected options should create five tasks");
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

    expect (myworld::hasStartupProofRequest (all), "all options should request startup proof");
    expect (myworld::startupProofTasks (all).size() == 21, "all options should create every startup proof task");

    return 0;
}
