#pragma once

#include <vector>

namespace myworld
{
enum class StartupProofTaskId
{
    v1Shader,
    a1Audio,
    liveIO,
    c2Storage,
    c3SaveWork,
    c4AIWorkerSaveWork,
    c5ModulePublish,
    c5AIWorkerModulePublish,
    c5VisibleModulePublish,
    c6AnalyzerFamily,
    c6AIRepairLoop,
    pvAttackDetector,
    pvDensityDetector,
    pvSilenceDetector,
    pvSustainDetector,
    pvResidueDetector,
    pvAggregatePressure,
    pvB1AnalyzerEnvironment,
    app1WorkbenchSession,
    app2WorkbenchOpenStatus
};

struct StartupProofTask
{
    StartupProofTaskId id;
    int delayMilliseconds;
};

struct StartupProofOptions
{
    bool dumpV1ShaderProof = false;
    bool dumpA1AudioProof = false;
    bool dumpLiveIOProof = false;
    bool dumpC2StorageProof = false;
    bool dumpC3SaveWorkProof = false;
    bool dumpC4AIWorkerSaveWorkProof = false;
    bool dumpC5ModulePublishProof = false;
    bool dumpC5AIWorkerModulePublishProof = false;
    bool dumpC5VisibleModulePublishProof = false;
    bool dumpC6AnalyzerFamilyProof = false;
    bool dumpC6AIRepairLoopProof = false;
    bool dumpPVAttackDetectorProof = false;
    bool dumpPVDensityDetectorProof = false;
    bool dumpPVSilenceDetectorProof = false;
    bool dumpPVSustainDetectorProof = false;
    bool dumpPVResidueDetectorProof = false;
    bool dumpPVAggregatePressureProof = false;
    bool dumpPVB1AnalyzerEnvironmentProof = false;
    bool dumpAPP1WorkbenchSessionProof = false;
    bool dumpAPP2WorkbenchOpenStatusProof = false;
    bool quitAfterStartupDump = false;
};

bool hasStartupProofRequest (const StartupProofOptions& options);
std::vector<StartupProofTask> startupProofTasks (const StartupProofOptions& options);
}
