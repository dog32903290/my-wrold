#include "C5ModulePublishProofRunner.h"

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

std::string readTextFile (const std::filesystem::path& path)
{
    std::ifstream input (path, std::ios::binary);
    return { std::istreambuf_iterator<char> (input), std::istreambuf_iterator<char>() };
}

myworld::C5ModulePublishProofRunResult runProof (myworld::C5ModulePublishProofKind kind,
                                                 const std::filesystem::path& outputDirectory)
{
    myworld::C5ModulePublishProofRunRequest request;
    request.kind = kind;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };

    std::filesystem::remove_all (outputDirectory);
    return myworld::runC5ModulePublishProof (request);
}
}

int main()
{
    const auto root = std::filesystem::temp_directory_path() / "my-world-c5-module-publish-proof-runner-test";
    std::filesystem::remove_all (root);

    const auto moduleResult = runProof (myworld::C5ModulePublishProofKind::modulePublish,
                                        root / "c5-module-publish-proof");
    expect (moduleResult.ok, moduleResult.error);
    expect (moduleResult.status == "dumped", "module publish status");
    expect (moduleResult.reportPath == root / "c5-module-publish-proof" / "module_publish_report.json",
            "module publish report path");
    expect (std::filesystem::exists (moduleResult.reportPath), "module publish report exists");
    auto report = readTextFile (moduleResult.reportPath);
    expect (report.find ("\"kind\": \"c5ModulePublishProof\"") != std::string::npos, "module report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "module report ok");
    expect (report.find ("\"publishedNodeType\": \"compound.published-loudness\"") != std::string::npos,
            "module published node type");
    expect (report.find ("\"packageReloaded\": true") != std::string::npos, "module package reloaded");
    expect (report.find ("\"libraryReloaded\": true") != std::string::npos, "module library reloaded");
    expect (report.find ("\"runtimeCoverageStatus\": \"ready\"") != std::string::npos,
            "module runtime coverage");
    expect (report.find ("\"createdPublishedNode\": true") != std::string::npos,
            "module create proof");

    const auto aiResult = runProof (myworld::C5ModulePublishProofKind::aiWorkerModulePublish,
                                    root / "c5-ai-worker-module-publish-proof");
    expect (aiResult.ok, aiResult.error);
    expect (aiResult.status == "dumped", "AI worker publish status");
    expect (aiResult.reportPath == root / "c5-ai-worker-module-publish-proof"
                                      / "ai_worker_module_publish_report.json",
            "AI worker report path");
    expect (std::filesystem::exists (aiResult.reportPath), "AI worker report exists");
    report = readTextFile (aiResult.reportPath);
    expect (report.find ("\"kind\": \"c5AIWorkerModulePublishProof\"") != std::string::npos,
            "AI worker report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "AI worker report ok");
    expect (report.find ("\"allowedPublishModule\": true") != std::string::npos,
            "AI worker allowed publish");
    expect (report.find ("\"publishedNodeType\": \"compound.ai-published-loudness\"") != std::string::npos,
            "AI worker published node type");
    expect (report.find ("\"publishCommandLogStatus\": \"publish_module:published\"") != std::string::npos,
            "AI worker publish command log");
    expect (report.find ("\"aiCommandLogStatus\": \"ai_worker:publish_module:published\"") != std::string::npos,
            "AI worker command log");
    expect (report.find ("\"collaborationLogEntries\": 2") != std::string::npos,
            "AI worker collaboration log count");

    const auto visibleResult = runProof (myworld::C5ModulePublishProofKind::visibleModulePublish,
                                         root / "c5-visible-module-publish-proof");
    expect (visibleResult.ok, visibleResult.error);
    expect (visibleResult.status == "dumped", "visible publish status");
    expect (visibleResult.reportPath == root / "c5-visible-module-publish-proof"
                                           / "visible_module_publish_report.json",
            "visible report path");
    expect (std::filesystem::exists (visibleResult.reportPath), "visible report exists");
    report = readTextFile (visibleResult.reportPath);
    expect (report.find ("\"kind\": \"c5VisibleModulePublishProof\"") != std::string::npos,
            "visible report kind");
    expect (report.find ("\"ok\": true") != std::string::npos, "visible report ok");
    expect (report.find ("\"publishedNodeType\": \"compound.visible-loud1\"") != std::string::npos,
            "visible published node type");
    expect (report.find ("\"packageReloaded\": true") != std::string::npos, "visible package reloaded");
    expect (report.find ("\"libraryReloaded\": true") != std::string::npos, "visible library reloaded");
    expect (report.find ("\"visibleRegistryContainsPublishedNode\": true") != std::string::npos,
            "visible registry proof");
    expect (report.find ("\"createdPublishedNode\": true") != std::string::npos,
            "visible create proof");

    std::filesystem::remove_all (root);
    std::cout << "c5 module publish proof runner ok\n";
    return 0;
}
