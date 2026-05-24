#pragma once

#include "AIWorkerCommand.h"
#include "InteractionContract.h"
#include "RuntimeRegistry.h"
#include "StorageCommand.h"

#include <cstddef>
#include <string>
#include <vector>

namespace myworld
{
std::string makeC2StorageReportJson (bool ok,
                                     const std::string& workManifestPath,
                                     const std::string& savedPatchPath,
                                     const std::string& saveStatus,
                                     const GraphSession& session,
                                     bool publicInputEdge,
                                     bool publicOutputEdge,
                                     bool monoMixLayout,
                                     double monoMixX,
                                     double monoMixY,
                                     const std::string& error);

std::string makeC3SaveWorkReportJson (bool ok,
                                      const std::string& workManifestPath,
                                      const std::string& savedPatchPath,
                                      const std::string& saveLogPath,
                                      const std::string& saveStatus,
                                      const std::string& commandLogStatus,
                                      const SaveLogLoadResult& saveLog,
                                      const GraphSession& session,
                                      bool publicInputEdge,
                                      bool publicOutputEdge,
                                      bool monoMixLayout,
                                      double monoMixX,
                                      double monoMixY,
                                      const std::string& error);

std::string makeC4AIWorkerSaveWorkReportJson (bool ok,
                                              const AIWorkerCommandRequest& moveRequest,
                                              const AIWorkerCommandResult& moveResult,
                                              const AIWorkerCommandRequest& saveRequest,
                                              const AIWorkerCommandResult& saveResult,
                                              const std::vector<std::string>& allowedOperations,
                                              const SaveLogLoadResult& saveLog,
                                              const GraphSession& session,
                                              bool publicInputEdge,
                                              bool publicOutputEdge,
                                              bool monoMixLayout,
                                              double monoMixX,
                                              double monoMixY,
                                              bool savedMovePersisted,
                                              double savedMoveX,
                                              double savedMoveY,
                                              const std::string& aiCommandLogStatus,
                                              const std::string& error);

std::string makeC5ModulePublishReportJson (bool ok,
                                           const PublishModuleResult& publish,
                                           bool packageReloaded,
                                           bool libraryReloaded,
                                           bool visibleRegistryContainsPublishedNode,
                                           bool runtimeRegistryContainsPublishedNode,
                                           const std::string& runtimeCoverageStatus,
                                           bool createdPublishedNode,
                                           const std::string& graphCommandLogStatus,
                                           const std::string& error);

std::string makeC5AIWorkerModulePublishReportJson (bool ok,
                                                   bool allowedPublishModule,
                                                   const AIWorkerCommandRequest& request,
                                                   const AIWorkerCommandResult& result,
                                                   size_t collaborationLogEntries,
                                                   const std::string& collaborationProofEvidence,
                                                   const std::string& aiCommandLogStatus,
                                                   const std::string& error);

std::string makeC5VisibleModulePublishReportJson (bool ok,
                                                  const PublishModuleResult& publish,
                                                  bool packageReloaded,
                                                  bool libraryReloaded,
                                                  bool visibleRegistryContainsPublishedNode,
                                                  bool createdPublishedNode,
                                                  const std::string& graphCommandLogStatus,
                                                  const std::string& error);

std::string makeC6AnalyzerFamilyReportJson (bool ok,
                                            const std::string& libraryPath,
                                            size_t familyEntryCount,
                                            bool visibleRegistryContainsRawEnergy,
                                            bool runtimeRegistryContainsRawEnergy,
                                            const std::string& runtimeCoverageStatus,
                                            bool createdRawEnergyNode,
                                            const std::string& graphCommandLogStatus,
                                            bool loudnessStillPresent,
                                            const std::vector<RuntimeOutputValue>& rawEnergyPublicOutputs,
                                            const std::string& error);

std::string makeC6AIRepairLoopReportJson (bool ok,
                                          const AIWorkerRepairLoopResult& repairResult,
                                          bool graphMutationApplied,
                                          size_t collaborationLogEntries,
                                          bool usesInteractionState,
                                          double finalNodeX,
                                          double finalNodeY,
                                          const std::string& error);
}
