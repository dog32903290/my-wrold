#include "StartupProof.h"

namespace myworld
{
bool hasStartupProofRequest (const StartupProofOptions& options)
{
    return options.dumpV1ShaderProof
           || options.dumpA1AudioProof
           || options.dumpLiveIOProof
           || options.dumpC2StorageProof
           || options.dumpC3SaveWorkProof
           || options.dumpC4AIWorkerSaveWorkProof
           || options.dumpC5ModulePublishProof
           || options.dumpC5AIWorkerModulePublishProof
           || options.dumpC5VisibleModulePublishProof
           || options.dumpC6AnalyzerFamilyProof
           || options.dumpC6AIRepairLoopProof
           || options.dumpPVAttackDetectorProof
           || options.dumpPVDensityDetectorProof
           || options.dumpPVSilenceDetectorProof
           || options.dumpPVSustainDetectorProof
           || options.dumpPVResidueDetectorProof
           || options.dumpPVAggregatePressureProof
           || options.dumpPVB1AnalyzerEnvironmentProof
           || options.dumpAPP1WorkbenchSessionProof
           || options.dumpAPP2WorkbenchOpenStatusProof
           || options.dumpAPPWorkbenchSessionProof
           || options.dumpProjectCreationProof
           || options.dumpCreatedProjectOpenProof
           || options.dumpAppSaveProof
           || options.dumpAppStatusProof
           || options.dumpWorkbenchStatusSurfaceProof
           || options.dumpWorkbenchGraphSurfaceProof
           || options.dumpWorkbenchCanvasSurfaceProof
           || options.dumpWorkbenchRuntimeSurfaceProof;
}

std::vector<StartupProofTask> startupProofTasks (const StartupProofOptions& options)
{
    std::vector<StartupProofTask> tasks;

    const auto addTask = [&tasks] (bool enabled, StartupProofTaskId id, int delayMilliseconds)
    {
        if (enabled)
            tasks.push_back ({ id, delayMilliseconds });
    };

    addTask (options.dumpV1ShaderProof, StartupProofTaskId::v1Shader, 750);
    addTask (options.dumpA1AudioProof, StartupProofTaskId::a1Audio, 2500);
    addTask (options.dumpLiveIOProof, StartupProofTaskId::liveIO, 500);
    addTask (options.dumpC2StorageProof, StartupProofTaskId::c2Storage, 500);
    addTask (options.dumpC3SaveWorkProof, StartupProofTaskId::c3SaveWork, 500);
    addTask (options.dumpC4AIWorkerSaveWorkProof, StartupProofTaskId::c4AIWorkerSaveWork, 500);
    addTask (options.dumpC5ModulePublishProof, StartupProofTaskId::c5ModulePublish, 500);
    addTask (options.dumpC5AIWorkerModulePublishProof, StartupProofTaskId::c5AIWorkerModulePublish, 500);
    addTask (options.dumpC5VisibleModulePublishProof, StartupProofTaskId::c5VisibleModulePublish, 500);
    addTask (options.dumpC6AnalyzerFamilyProof, StartupProofTaskId::c6AnalyzerFamily, 500);
    addTask (options.dumpC6AIRepairLoopProof, StartupProofTaskId::c6AIRepairLoop, 500);
    addTask (options.dumpPVAttackDetectorProof, StartupProofTaskId::pvAttackDetector, 500);
    addTask (options.dumpPVDensityDetectorProof, StartupProofTaskId::pvDensityDetector, 500);
    addTask (options.dumpPVSilenceDetectorProof, StartupProofTaskId::pvSilenceDetector, 500);
    addTask (options.dumpPVSustainDetectorProof, StartupProofTaskId::pvSustainDetector, 500);
    addTask (options.dumpPVResidueDetectorProof, StartupProofTaskId::pvResidueDetector, 500);
    addTask (options.dumpPVAggregatePressureProof, StartupProofTaskId::pvAggregatePressure, 500);
    addTask (options.dumpPVB1AnalyzerEnvironmentProof, StartupProofTaskId::pvB1AnalyzerEnvironment, 500);
    addTask (options.dumpAPP1WorkbenchSessionProof, StartupProofTaskId::app1WorkbenchSession, 500);
    addTask (options.dumpAPP2WorkbenchOpenStatusProof, StartupProofTaskId::app2WorkbenchOpenStatus, 500);
    addTask (options.dumpAPPWorkbenchSessionProof, StartupProofTaskId::appWorkbenchSession, 500);
    addTask (options.dumpProjectCreationProof, StartupProofTaskId::projectCreation, 500);
    addTask (options.dumpCreatedProjectOpenProof, StartupProofTaskId::createdProjectOpen, 500);
    addTask (options.dumpAppSaveProof, StartupProofTaskId::appSave, 500);
    addTask (options.dumpAppStatusProof, StartupProofTaskId::appStatus, 500);
    addTask (options.dumpWorkbenchStatusSurfaceProof, StartupProofTaskId::workbenchStatusSurface, 500);
    addTask (options.dumpWorkbenchGraphSurfaceProof, StartupProofTaskId::workbenchGraphSurface, 500);
    addTask (options.dumpWorkbenchCanvasSurfaceProof, StartupProofTaskId::workbenchCanvasSurface, 500);
    addTask (options.dumpWorkbenchRuntimeSurfaceProof, StartupProofTaskId::workbenchRuntimeSurface, 500);

    return tasks;
}
}
