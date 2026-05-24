#include "PVDetectorProofRunner.h"

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

std::string readTextFile (const std::filesystem::path& path)
{
    std::ifstream input (path);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

void expectArtifact (const std::filesystem::path& directory, const std::string& name)
{
    expect (std::filesystem::exists (directory / name), "missing artifact " + name);
}
}

int main()
{
    const auto root = std::filesystem::current_path();
    const auto outputRoot = std::filesystem::temp_directory_path() / "my-world-pv-proof-runner-tests";
    std::filesystem::remove_all (outputRoot);

    myworld::PVDetectorProofRunRequest attackRequest;
    attackRequest.kind = myworld::PVDetectorProofKind::attack;
    attackRequest.outputDirectory = outputRoot / "attack";
    attackRequest.candidateRoots = { root };

    const auto attack = myworld::runPVDetectorProof (attackRequest);
    expect (attack.ok, attack.error);
    expect (attack.status == "dumped", "attack proof status");
    expect (attack.reportPath.filename() == "attack_detector_report.json", "attack report filename");
    expectArtifact (attackRequest.outputDirectory, "attack_detector_report.json");
    expectArtifact (attackRequest.outputDirectory, "cook_order.json");
    expectArtifact (attackRequest.outputDirectory, "node_stats.json");
    expectArtifact (attackRequest.outputDirectory, "errors.json");

    const auto attackReport = readTextFile (attackRequest.outputDirectory / "attack_detector_report.json");
    expect (attackReport.find ("\"kind\": \"pvAttackDetectorProof\"") != std::string::npos,
            "attack report kind");
    expect (attackReport.find ("\"operation\": \"pv_attack_detector\"") != std::string::npos,
            "attack report operation");

    myworld::PVDetectorProofRunRequest aggregateRequest;
    aggregateRequest.kind = myworld::PVDetectorProofKind::aggregatePressure;
    aggregateRequest.outputDirectory = outputRoot / "aggregate-pressure";
    aggregateRequest.candidateRoots = { root };

    const auto aggregate = myworld::runPVDetectorProof (aggregateRequest);
    expect (aggregate.ok, aggregate.error);
    expect (aggregate.status == "dumped", "aggregate proof status");
    expect (aggregate.reportPath.filename() == "aggregate_pressure_report.json", "aggregate report filename");
    expectArtifact (aggregateRequest.outputDirectory, "aggregate_pressure_report.json");
    expectArtifact (aggregateRequest.outputDirectory, "cook_order.json");
    expectArtifact (aggregateRequest.outputDirectory, "node_stats.json");
    expectArtifact (aggregateRequest.outputDirectory, "errors.json");

    const auto aggregateReport = readTextFile (aggregateRequest.outputDirectory / "aggregate_pressure_report.json");
    expect (aggregateReport.find ("\"kind\": \"pvAggregatePressureProof\"") != std::string::npos,
            "aggregate report kind");
    expect (aggregateReport.find ("\"operation\": \"pv_aggregate_pressure\"") != std::string::npos,
            "aggregate report operation");

    std::filesystem::remove_all (outputRoot);
    std::cout << "pv detector proof runner ok\n";
    return 0;
}
