#include "LiveIOProofRunner.h"

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

void expectContains (const std::string& text, const std::string& expected, const std::string& message)
{
    expect (text.find (expected) != std::string::npos, message + " should contain " + expected);
}
}

int main()
{
    const auto outputDirectory = std::filesystem::temp_directory_path()
                                 / "my-world-live-io-proof-runner-test";
    std::filesystem::remove_all (outputDirectory);

    myworld::LiveIOProofRunRequest request;
    request.outputDirectory = outputDirectory;
    request.candidateRoots = { std::filesystem::current_path() };
    request.loudness = 0.5f;
    request.midiOutputInventory.devices = {
        { "IAC Driver Bus 1", "iac-1" }
    };

    const auto result = myworld::runLiveIOProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.outputDirectory == outputDirectory, "output directory");
    expect (result.artifactPaths.size() == 5, "artifact count");
    expect (std::filesystem::exists (outputDirectory / "live_io_report.json"),
            "live io report exists");
    expect (std::filesystem::exists (outputDirectory / "live_io_send_report.json"),
            "live io send report exists");
    expect (std::filesystem::exists (outputDirectory / "live_io_osc_loopback_report.json"),
            "live io osc loopback report exists");
    expect (std::filesystem::exists (outputDirectory / "live_io_midi_inventory_report.json"),
            "live io midi inventory report exists");
    expect (std::filesystem::exists (outputDirectory / "live_io_runtime_execution.json"),
            "runtime execution exists");

    const auto report = readTextFile (outputDirectory / "live_io_report.json");
    expectContains (report, "\"kind\": \"liveIOProof\"", "proof report kind");
    expectContains (report, "\"ok\": true", "proof report ok");
    expectContains (report, "\"runtimeSource\": \"compound.loudness.publicOutputs\"", "proof report source");
    expectContains (report, "\"targetKind\": \"midi.cc\"", "proof report midi target");
    expectContains (report, "\"targetKind\": \"osc.float\"", "proof report osc target");
    expectContains (report, "\"targetKind\": \"shader.uniform\"", "proof report uniform target");
    expectContains (report, "\"channel\": 1", "proof report midi channel");
    expectContains (report, "\"cc\": 20", "proof report midi cc");
    expectContains (report, "\"oscAddress\": \"/my-world/loudness\"", "proof report osc address");
    expectContains (report, "\"uniformName\": \"u_loudness\"", "proof report uniform name");
    expectContains (report, "\"errors\": []", "proof report no errors");

    const auto sendReport = readTextFile (outputDirectory / "live_io_send_report.json");
    expectContains (sendReport, "\"kind\": \"liveIOSendReport\"", "send report kind");
    expectContains (sendReport, "\"ok\": true", "send report ok");
    expectContains (sendReport, "\"status\": \"dry_run\"", "send report status");
    expectContains (sendReport, "\"targetKind\": \"midi.cc\"", "send report midi action");
    expectContains (sendReport, "\"targetKind\": \"osc.float\"", "send report osc action");
    expectContains (sendReport, "\"midiOutputName\": \"dry-run MIDI\"", "send report midi route");
    expectContains (sendReport, "\"oscHost\": \"127.0.0.1\"", "send report osc host");
    expectContains (sendReport, "\"oscPort\": 9000", "send report osc port");
    expectContains (sendReport, "\"sent\": false", "send report dry-run send flag");
    expectContains (sendReport, "\"skipped\": [\"shader.uniform: uniform.loudness\"]",
                    "send report skipped uniform");
    expectContains (sendReport, "\"errors\": []", "send report no errors");

    const auto loopbackReport = readTextFile (outputDirectory / "live_io_osc_loopback_report.json");
    expectContains (loopbackReport, "\"kind\": \"liveIOOscLoopbackProof\"", "loopback report kind");
    expectContains (loopbackReport, "\"ok\": true", "loopback report ok");
    expectContains (loopbackReport, "\"status\": \"received\"", "loopback report status");
    expectContains (loopbackReport, "\"sendStatus\": \"controlled_send\"", "loopback send status");
    expectContains (loopbackReport, "\"sent\": true", "loopback sent flag");
    expectContains (loopbackReport, "\"received\": true", "loopback received flag");
    expectContains (loopbackReport, "\"oscHost\": \"127.0.0.1\"", "loopback host");
    expectContains (loopbackReport, "\"oscAddress\": \"/my-world/loudness\"", "loopback address");
    expectContains (loopbackReport, "\"receivedFloatValue\": 0.500000", "loopback value");
    expectContains (loopbackReport, "\"errors\": []", "loopback no errors");

    const auto midiInventoryReport = readTextFile (outputDirectory / "live_io_midi_inventory_report.json");
    expectContains (midiInventoryReport, "\"kind\": \"liveIOMidiOutputInventoryProof\"",
                    "midi inventory kind");
    expectContains (midiInventoryReport, "\"ok\": true", "midi inventory ok");
    expectContains (midiInventoryReport, "\"deviceCount\": 1", "midi inventory count");
    expectContains (midiInventoryReport, "\"name\": \"IAC Driver Bus 1\"", "midi inventory device");
    expectContains (midiInventoryReport, "\"identifier\": \"iac-1\"", "midi inventory identifier");
    expectContains (midiInventoryReport, "\"selectedRoute\": {", "midi selected route");
    expectContains (midiInventoryReport, "\"status\": \"selected\"", "midi selected route");
    expectContains (midiInventoryReport, "\"unavailableRoute\": {", "midi unavailable route");
    expectContains (midiInventoryReport, "\"status\": \"unavailable\"", "midi unavailable route");
    expectContains (midiInventoryReport, "\"errors\": [\"midi output is unavailable: __missing_live_io_midi_output__\"]",
                    "midi unavailable error");

    const auto runtimeExecution = readTextFile (outputDirectory / "live_io_runtime_execution.json");
    expectContains (runtimeExecution, "\"kind\": \"runtimeExecution\"", "runtime execution kind");
    expectContains (runtimeExecution, "\"nodeType\": \"compound.loudness\"", "runtime execution node");
    expectContains (runtimeExecution, "\"status\": \"computed\"", "runtime execution status");
    expectContains (runtimeExecution, "\"publicOutputs\": {", "runtime public outputs");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "live io proof runner ok\n";
    return 0;
}
