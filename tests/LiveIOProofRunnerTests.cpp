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
    request.midiOutputSender = [] (const myworld::LiveIOMidiOutputDevice&,
                                   const myworld::LiveIOMidiCcMessage&)
    {
        return myworld::LiveIOMidiOutputDeviceSendResult { true, true, "" };
    };
    request.controlOscSender = [] (const myworld::LiveIOOscFloatMessage&)
    {
        return myworld::LiveIOOscFloatSendResult { true, "" };
    };

    const auto result = myworld::runLiveIOProof (request);

    expect (result.ok, result.error);
    expect (result.status == "dumped", "status");
    expect (result.outputDirectory == outputDirectory, "output directory");
    expect (result.artifactPaths.size() == 11, "artifact count");
    expect (std::filesystem::exists (outputDirectory / "live_io_report.json"),
            "live io report exists");
    expect (std::filesystem::exists (outputDirectory / "live_io_send_report.json"),
            "live io send report exists");
    expect (std::filesystem::exists (outputDirectory / "live_io_osc_loopback_report.json"),
            "live io osc loopback report exists");
    expect (std::filesystem::exists (outputDirectory / "live_io_midi_inventory_report.json"),
            "live io midi inventory report exists");
    expect (std::filesystem::exists (outputDirectory / "live_io_midi_send_report.json"),
            "live io midi send report exists");
    expect (std::filesystem::exists (outputDirectory / "live_io_control_dispatch_report.json"),
            "live io control dispatch report exists");
    expect (std::filesystem::exists (outputDirectory / "live_io_control_pump_report.json"),
            "live io control pump report exists");
    expect (std::filesystem::exists (outputDirectory / "live_io_app_timer_midi_report.json"),
            "live io app timer midi report exists");
    expect (std::filesystem::exists (outputDirectory / "live_io_app_timer_osc_loopback_report.json"),
            "live io app timer osc loopback report exists");
    expect (std::filesystem::exists (outputDirectory / "live_io_shader_uniform_report.json"),
            "live io shader uniform report exists");
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

    const auto midiSendReport = readTextFile (outputDirectory / "live_io_midi_send_report.json");
    expectContains (midiSendReport, "\"kind\": \"liveIOMidiOutputSendProof\"",
                    "midi send kind");
    expectContains (midiSendReport, "\"ok\": true", "midi send ok");
    expectContains (midiSendReport, "\"status\": \"sent\"", "midi send status");
    expectContains (midiSendReport, "\"selectedIdentifier\": \"iac-1\"", "midi send identifier");
    expectContains (midiSendReport, "\"opened\": true", "midi send opened");
    expectContains (midiSendReport, "\"sent\": true", "midi send sent");
    expectContains (midiSendReport, "\"bindingId\": \"midi.loudness\"", "midi send binding");
    expectContains (midiSendReport, "\"channel\": 1", "midi send channel");
    expectContains (midiSendReport, "\"cc\": 20", "midi send cc");
    expectContains (midiSendReport, "\"value\": 64", "midi send value");
    expectContains (midiSendReport, "\"statusByte\": 176", "midi send status byte");
    expectContains (midiSendReport, "\"data1\": 20", "midi send data1");
    expectContains (midiSendReport, "\"data2\": 64", "midi send data2");
    expectContains (midiSendReport, "\"errors\": []", "midi send no errors");

    const auto controlDispatchReport = readTextFile (outputDirectory / "live_io_control_dispatch_report.json");
    expectContains (controlDispatchReport, "\"kind\": \"liveIOControlDispatchProof\"",
                    "control dispatch kind");
    expectContains (controlDispatchReport, "\"ok\": true", "control dispatch ok");
    expectContains (controlDispatchReport, "\"status\": \"dispatched\"", "control dispatch status");
    expectContains (controlDispatchReport, "\"frameCount\": 4", "control dispatch frame count");
    expectContains (controlDispatchReport, "\"dispatchedFrameCount\": 3",
                    "control dispatch dispatched count");
    expectContains (controlDispatchReport, "\"rateLimitedFrameCount\": 1",
                    "control dispatch rate limit count");
    expectContains (controlDispatchReport, "\"midiSentCount\": 3", "control dispatch midi count");
    expectContains (controlDispatchReport, "\"oscSentCount\": 3", "control dispatch osc count");
    expectContains (controlDispatchReport, "\"shaderSkippedCount\": 3",
                    "control dispatch shader skipped count");
    expectContains (controlDispatchReport, "\"status\": \"rate_limited\"",
                    "control dispatch rate limited frame");
    expectContains (controlDispatchReport, "\"errors\": []", "control dispatch no errors");

    const auto controlPumpReport = readTextFile (outputDirectory / "live_io_control_pump_report.json");
    expectContains (controlPumpReport, "\"kind\": \"liveIOControlPumpProof\"",
                    "control pump kind");
    expectContains (controlPumpReport, "\"ok\": true", "control pump ok");
    expectContains (controlPumpReport, "\"status\": \"pumped\"", "control pump status");
    expectContains (controlPumpReport, "\"tickCount\": 5", "control pump tick count");
    expectContains (controlPumpReport, "\"frameCount\": 3", "control pump frame count");
    expectContains (controlPumpReport, "\"inactiveTickCount\": 1", "control pump inactive count");
    expectContains (controlPumpReport, "\"tickRateLimitedCount\": 1",
                    "control pump tick rate limit count");
    expectContains (controlPumpReport, "\"lastSampleCounter\": 320",
                    "control pump sample counter");
    expectContains (controlPumpReport, "\"dispatch\": {", "control pump dispatch");
    expectContains (controlPumpReport, "\"midiSentCount\": 3", "control pump midi count");
    expectContains (controlPumpReport, "\"oscSentCount\": 3", "control pump osc count");
    expectContains (controlPumpReport, "\"status\": \"tick_rate_limited\"",
                    "control pump tick rate limited");
    expectContains (controlPumpReport, "\"status\": \"inactive\"", "control pump inactive");
    expectContains (controlPumpReport, "\"errors\": []", "control pump no errors");

    const auto appTimerMidiReport = readTextFile (outputDirectory / "live_io_app_timer_midi_report.json");
    expectContains (appTimerMidiReport, "\"kind\": \"liveIOAppTimerMidiProof\"",
                    "app timer midi kind");
    expectContains (appTimerMidiReport, "\"ok\": true", "app timer midi ok");
    expectContains (appTimerMidiReport, "\"status\": \"controlled_sent\"",
                    "app timer midi status");
    expectContains (appTimerMidiReport, "\"sendMode\": \"controlled_send\"",
                    "app timer midi send mode");
    expectContains (appTimerMidiReport, "\"outputOperator\": \"midi.cc\"",
                    "app timer midi output operator");
    expectContains (appTimerMidiReport, "\"midiControlledSendCount\": 1",
                    "app timer midi controlled count");
    expectContains (appTimerMidiReport, "\"oscControlledSendCount\": 0",
                    "app timer midi osc count");
    expectContains (appTimerMidiReport, "\"shaderSkippedCount\": 1",
                    "app timer midi shader skip");
    expectContains (appTimerMidiReport, "\"hasLastShaderUniform\": true",
                    "app timer midi has shader uniform");
    expectContains (appTimerMidiReport, "\"shaderUniformEvidence\": {",
                    "app timer midi shader uniform evidence object");
    expectContains (appTimerMidiReport, "\"source\": \"LiveIOControlTimerState\"",
                    "app timer midi shader uniform evidence source");
    expectContains (appTimerMidiReport, "\"lastShaderUniformName\": \"u_loudness\"",
                    "app timer midi shader uniform name");
    expectContains (appTimerMidiReport, "\"lastShaderUniformValue\": 0.500000",
                    "app timer midi shader uniform value");
    expectContains (appTimerMidiReport, "\"lastShaderUniformSampleCounter\": 64",
                    "app timer midi shader uniform sample");
    expectContains (appTimerMidiReport, "\"lastSampleCounter\": 64",
                    "app timer midi sample counter");
    expectContains (appTimerMidiReport, "\"errors\": []", "app timer midi no errors");

    const auto appTimerOscReport = readTextFile (outputDirectory / "live_io_app_timer_osc_loopback_report.json");
    expectContains (appTimerOscReport, "\"kind\": \"liveIOAppTimerOscLoopbackProof\"",
                    "app timer osc kind");
    expectContains (appTimerOscReport, "\"ok\": true", "app timer osc ok");
    expectContains (appTimerOscReport, "\"status\": \"received\"", "app timer osc status");
    expectContains (appTimerOscReport, "\"timerStatus\": \"controlled_sent\"",
                    "app timer osc timer status");
    expectContains (appTimerOscReport, "\"sendMode\": \"controlled_send\"",
                    "app timer osc send mode");
    expectContains (appTimerOscReport, "\"outputOperator\": \"osc.float\"",
                    "app timer osc output operator");
    expectContains (appTimerOscReport, "\"midiControlledSendCount\": 0",
                    "app timer osc midi count");
    expectContains (appTimerOscReport, "\"oscControlledSendCount\": 1",
                    "app timer osc controlled count");
    expectContains (appTimerOscReport, "\"shaderSkippedCount\": 1",
                    "app timer osc shader skip");
    expectContains (appTimerOscReport, "\"hasLastShaderUniform\": true",
                    "app timer osc has shader uniform");
    expectContains (appTimerOscReport, "\"shaderUniformEvidence\": {",
                    "app timer osc shader uniform evidence object");
    expectContains (appTimerOscReport, "\"source\": \"LiveIOControlTimerState\"",
                    "app timer osc shader uniform evidence source");
    expectContains (appTimerOscReport, "\"lastShaderUniformName\": \"u_loudness\"",
                    "app timer osc shader uniform name");
    expectContains (appTimerOscReport, "\"lastShaderUniformValue\": 0.500000",
                    "app timer osc shader uniform value");
    expectContains (appTimerOscReport, "\"received\": true", "app timer osc received");
    expectContains (appTimerOscReport, "\"oscHost\": \"127.0.0.1\"", "app timer osc host");
    expectContains (appTimerOscReport, "\"oscAddress\": \"/my-world/loudness\"",
                    "app timer osc address");
    expectContains (appTimerOscReport, "\"receivedFloatValue\": 0.500000",
                    "app timer osc value");
    expectContains (appTimerOscReport, "\"errors\": []", "app timer osc no errors");

    const auto shaderUniformReport = readTextFile (outputDirectory / "live_io_shader_uniform_report.json");
    expectContains (shaderUniformReport, "\"kind\": \"liveIOShaderUniformProof\"",
                    "shader uniform report kind");
    expectContains (shaderUniformReport, "\"ok\": true", "shader uniform report ok");
    expectContains (shaderUniformReport, "\"status\": \"captured\"", "shader uniform report status");
    expectContains (shaderUniformReport, "\"uniformName\": \"u_loudness\"",
                    "shader uniform report name");
    expectContains (shaderUniformReport, "\"value\": 0.500000", "shader uniform report value");
    expectContains (shaderUniformReport, "\"sampleCounter\": 64", "shader uniform report sample");
    expectContains (shaderUniformReport, "\"errors\": []", "shader uniform report no errors");

    const auto runtimeExecution = readTextFile (outputDirectory / "live_io_runtime_execution.json");
    expectContains (runtimeExecution, "\"kind\": \"runtimeExecution\"", "runtime execution kind");
    expectContains (runtimeExecution, "\"nodeType\": \"compound.loudness\"", "runtime execution node");
    expectContains (runtimeExecution, "\"status\": \"computed\"", "runtime execution status");
    expectContains (runtimeExecution, "\"publicOutputs\": {", "runtime public outputs");

    std::filesystem::remove_all (outputDirectory);
    std::cout << "live io proof runner ok\n";
    return 0;
}
