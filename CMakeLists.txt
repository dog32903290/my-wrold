cmake_minimum_required(VERSION 3.25)

project(MyWorld VERSION 0.1.0 LANGUAGES C CXX OBJC OBJCXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MY_WORLD_DISPLAY_NAME "我的世界")
option(MY_WORLD_FETCH_JUCE "Download JUCE with FetchContent when MY_WORLD_JUCE_DIR is not set" OFF)
set(MY_WORLD_JUCE_TAG "8.0.12" CACHE STRING "JUCE git tag used when MY_WORLD_FETCH_JUCE is enabled")
set(MY_WORLD_DEFAULT_JUCE_DIR "")
if(DEFINED ENV{MY_WORLD_JUCE_DIR})
    set(MY_WORLD_DEFAULT_JUCE_DIR "$ENV{MY_WORLD_JUCE_DIR}")
elseif(DEFINED ENV{JUCE_DIR})
    set(MY_WORLD_DEFAULT_JUCE_DIR "$ENV{JUCE_DIR}")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE/CMakeLists.txt")
    set(MY_WORLD_DEFAULT_JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE")
endif()
set(MY_WORLD_JUCE_DIR "${MY_WORLD_DEFAULT_JUCE_DIR}" CACHE PATH "Path to a JUCE checkout")
set(MY_WORLD_HAVE_JUCE OFF)

enable_testing()

add_library(my_world_json
    source/core/JsonWriter.cpp
)

target_include_directories(my_world_json
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

add_executable(my_world_json_writer_tests
    tests/JsonWriterTests.cpp
)

target_link_libraries(my_world_json_writer_tests
    PRIVATE
        my_world_json
)

add_test(NAME json_writer COMMAND my_world_json_writer_tests)

add_library(my_world_path_resolution
    source/core/PathResolution.cpp
)

target_include_directories(my_world_path_resolution
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

add_executable(my_world_path_resolution_tests
    tests/PathResolutionTests.cpp
)

target_link_libraries(my_world_path_resolution_tests
    PRIVATE
        my_world_path_resolution
)

add_test(NAME path_resolution COMMAND my_world_path_resolution_tests)
set_tests_properties(path_resolution PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_startup_proof
    source/app/StartupProof.cpp
)

target_include_directories(my_world_startup_proof
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
)

add_executable(my_world_startup_proof_tests
    tests/StartupProofTests.cpp
)

target_link_libraries(my_world_startup_proof_tests
    PRIVATE
        my_world_startup_proof
)

add_test(NAME startup_proof COMMAND my_world_startup_proof_tests)

add_library(my_world_core
    source/core/GraphContract.cpp
    source/core/GraphIOMapping.cpp
    source/core/LiveIOBus.cpp
    source/core/LiveIOControlDispatcher.cpp
    source/core/LiveIOControlPump.cpp
    source/core/LiveIOControlTimer.cpp
    source/core/LiveIOMidiTeach.cpp
    source/core/LiveIOMidiOutputInventory.cpp
    source/core/LiveIOMidiSendProof.cpp
    source/core/LiveIOOscReceiver.cpp
    source/core/LiveIOSendAdapter.cpp
    source/core/LiveIOStatusIndicator.cpp
    source/core/OutputViewState.cpp
    source/core/TimelineState.cpp
    source/core/VariationState.cpp
)

target_include_directories(my_world_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/audio
)

target_link_libraries(my_world_core
    PUBLIC
        my_world_json
)

add_executable(my_world_graph_contract_tests
    tests/GraphContractTests.cpp
)

target_link_libraries(my_world_graph_contract_tests
    PRIVATE
        my_world_core
)

add_test(NAME graph_contract COMMAND my_world_graph_contract_tests)

add_executable(my_world_render_backend_contract_tests
    tests/RenderBackendContractTests.cpp
)

target_include_directories(my_world_render_backend_contract_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/render
)

add_test(NAME render_backend_contract COMMAND my_world_render_backend_contract_tests)

add_library(my_world_shader_preview_input_bridge
    source/render/ShaderPreviewInputBridge.cpp
)

target_include_directories(my_world_shader_preview_input_bridge
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/render
)

add_executable(my_world_shader_preview_input_bridge_tests
    tests/ShaderPreviewInputBridgeTests.cpp
)

target_link_libraries(my_world_shader_preview_input_bridge_tests
    PRIVATE
        my_world_shader_preview_input_bridge
)

add_test(NAME shader_preview_input_bridge COMMAND my_world_shader_preview_input_bridge_tests)

add_executable(my_world_shader_preview_smoke_read_tests
    tests/ShaderPreviewSmokeReadTests.cpp
)

target_link_libraries(my_world_shader_preview_smoke_read_tests
    PRIVATE
        my_world_shader_preview_input_bridge
)

target_include_directories(my_world_shader_preview_smoke_read_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/render
)

add_test(NAME shader_preview_smoke_read COMMAND my_world_shader_preview_smoke_read_tests)

add_library(my_world_graph_language
    source/core/GraphLanguage.cpp
)

target_include_directories(my_world_graph_language
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

add_executable(my_world_graph_language_tests
    tests/GraphLanguageTests.cpp
)

target_link_libraries(my_world_graph_language_tests
    PRIVATE
        my_world_graph_language
)

add_test(NAME graph_language COMMAND my_world_graph_language_tests)
set_tests_properties(graph_language PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_node_specs
    source/core/NodeSpecBrowser.cpp
    source/core/NodeSpec.cpp
)

target_include_directories(my_world_node_specs
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

add_executable(my_world_node_spec_tests
    tests/NodeSpecTests.cpp
)

target_link_libraries(my_world_node_spec_tests
    PRIVATE
        my_world_node_specs
)

add_test(NAME node_specs COMMAND my_world_node_spec_tests)

add_executable(my_world_node_spec_browser_tests
    tests/NodeSpecBrowserTests.cpp
)

target_link_libraries(my_world_node_spec_browser_tests
    PRIVATE
        my_world_compound_module
        my_world_storage
)

add_test(NAME node_spec_browser COMMAND my_world_node_spec_browser_tests)
set_tests_properties(node_spec_browser PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_compound
    source/core/CompoundPatch.cpp
)

target_include_directories(my_world_compound
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

target_link_libraries(my_world_compound
    PUBLIC
        my_world_json
)

add_library(my_world_compound_module
    source/core/CompoundModule.cpp
)

target_include_directories(my_world_compound_module
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_compound_module
    PUBLIC
        my_world_compound
        my_world_node_specs
        my_world_path_resolution
        my_world_storage
)

add_library(my_world_runtime_registry
    source/core/RuntimeRegistryDiagnostics.cpp
    source/core/RuntimeRegistry.cpp
    source/core/RuntimeRegistryJson.cpp
    source/core/RuntimeRegistrySynthetic.cpp
)

target_include_directories(my_world_runtime_registry
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/audio
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_runtime_registry
    PUBLIC
        my_world_json
        my_world_audio
        my_world_compound_module
        my_world_path_resolution
)

add_executable(my_world_runtime_registry_tests
    tests/RuntimeRegistryTests.cpp
)

target_link_libraries(my_world_runtime_registry_tests
    PRIVATE
        my_world_runtime_registry
)

add_test(NAME runtime_registry COMMAND my_world_runtime_registry_tests)
set_tests_properties(runtime_registry PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(my_world_compound_patch_tests
    tests/CompoundPatchTests.cpp
)

target_link_libraries(my_world_compound_patch_tests
    PRIVATE
        my_world_compound
)

add_test(NAME compound_patch COMMAND my_world_compound_patch_tests)

add_library(my_world_patch_interaction
    source/core/PatchInteraction.cpp
)

target_include_directories(my_world_patch_interaction
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

add_executable(my_world_patch_interaction_tests
    tests/PatchInteractionTests.cpp
)

target_link_libraries(my_world_patch_interaction_tests
    PRIVATE
        my_world_patch_interaction
)

add_test(NAME patch_interaction COMMAND my_world_patch_interaction_tests)

add_library(my_world_interaction
    source/core/CanvasGeometry.cpp
    source/core/GraphEndpoint.cpp
    source/core/InteractionContract.cpp
    source/core/NodeSpecQueries.cpp
    source/core/ParameterControl.cpp
    source/core/ParameterRowState.cpp
)

target_include_directories(my_world_interaction
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

target_link_libraries(my_world_interaction
    PUBLIC
        my_world_core
        my_world_graph_language
        my_world_node_specs
        my_world_compound
)

add_executable(my_world_graph_endpoint_tests
    tests/GraphEndpointTests.cpp
)

target_link_libraries(my_world_graph_endpoint_tests
    PRIVATE
        my_world_interaction
)

add_test(NAME graph_endpoint COMMAND my_world_graph_endpoint_tests)

add_executable(my_world_canvas_geometry_contract_tests
    tests/CanvasGeometryContractTests.cpp
)

target_link_libraries(my_world_canvas_geometry_contract_tests
    PRIVATE
        my_world_interaction
)

add_test(NAME canvas_geometry_contract COMMAND my_world_canvas_geometry_contract_tests)

add_executable(my_world_node_spec_query_tests
    tests/NodeSpecQueryTests.cpp
)

target_link_libraries(my_world_node_spec_query_tests
    PRIVATE
        my_world_interaction
)

add_test(NAME node_spec_queries COMMAND my_world_node_spec_query_tests)

add_executable(my_world_compound_module_tests
    tests/CompoundModuleTests.cpp
)

target_link_libraries(my_world_compound_module_tests
    PRIVATE
        my_world_compound_module
        my_world_interaction
        my_world_storage
)

add_test(NAME compound_module COMMAND my_world_compound_module_tests)
set_tests_properties(compound_module PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(my_world_canvas_view_state_tests
    tests/CanvasViewStateTests.cpp
)

target_link_libraries(my_world_canvas_view_state_tests
    PRIVATE
        my_world_interaction
)

add_test(NAME canvas_view_state COMMAND my_world_canvas_view_state_tests)

add_executable(my_world_output_view_state_tests
    tests/OutputViewStateTests.cpp
)

target_link_libraries(my_world_output_view_state_tests
    PRIVATE
        my_world_storage_command
)

add_test(NAME output_view_state COMMAND my_world_output_view_state_tests)

add_executable(my_world_live_io_bus_tests
    tests/LiveIOBusTests.cpp
)

target_link_libraries(my_world_live_io_bus_tests
    PRIVATE
        my_world_core
)

add_test(NAME live_io_bus COMMAND my_world_live_io_bus_tests)

add_executable(my_world_graph_io_mapping_tests
    tests/GraphIOMappingTests.cpp
)

target_link_libraries(my_world_graph_io_mapping_tests
    PRIVATE
        my_world_storage
)

add_test(NAME graph_io_mapping COMMAND my_world_graph_io_mapping_tests)
set_tests_properties(graph_io_mapping PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(my_world_live_io_send_adapter_tests
    tests/LiveIOSendAdapterTests.cpp
)

target_link_libraries(my_world_live_io_send_adapter_tests
    PRIVATE
        my_world_core
)

add_test(NAME live_io_send_adapter COMMAND my_world_live_io_send_adapter_tests)

add_executable(my_world_live_io_midi_output_inventory_tests
    tests/LiveIOMidiOutputInventoryTests.cpp
)

target_link_libraries(my_world_live_io_midi_output_inventory_tests
    PRIVATE
        my_world_core
)

add_test(NAME live_io_midi_output_inventory COMMAND my_world_live_io_midi_output_inventory_tests)

add_executable(my_world_live_io_midi_send_proof_tests
    tests/LiveIOMidiSendProofTests.cpp
)

target_link_libraries(my_world_live_io_midi_send_proof_tests
    PRIVATE
        my_world_core
)

add_test(NAME live_io_midi_send_proof COMMAND my_world_live_io_midi_send_proof_tests)

add_executable(my_world_live_io_midi_teach_tests
    tests/LiveIOMidiTeachTests.cpp
)

target_link_libraries(my_world_live_io_midi_teach_tests
    PRIVATE
        my_world_core
)

add_test(NAME live_io_midi_teach COMMAND my_world_live_io_midi_teach_tests)

add_executable(my_world_live_io_osc_receiver_tests
    tests/LiveIOOscReceiverTests.cpp
)

target_link_libraries(my_world_live_io_osc_receiver_tests
    PRIVATE
        my_world_core
)

add_test(NAME live_io_osc_receiver COMMAND my_world_live_io_osc_receiver_tests)

add_executable(my_world_live_io_control_dispatcher_tests
    tests/LiveIOControlDispatcherTests.cpp
)

target_link_libraries(my_world_live_io_control_dispatcher_tests
    PRIVATE
        my_world_core
)

add_test(NAME live_io_control_dispatcher COMMAND my_world_live_io_control_dispatcher_tests)

add_executable(my_world_live_io_control_pump_tests
    tests/LiveIOControlPumpTests.cpp
)

target_link_libraries(my_world_live_io_control_pump_tests
    PRIVATE
        my_world_core
)

add_test(NAME live_io_control_pump COMMAND my_world_live_io_control_pump_tests)

add_executable(my_world_live_io_control_timer_tests
    tests/LiveIOControlTimerTests.cpp
)

target_link_libraries(my_world_live_io_control_timer_tests
    PRIVATE
        my_world_core
)

add_test(NAME live_io_control_timer COMMAND my_world_live_io_control_timer_tests)

add_executable(my_world_live_io_status_indicator_tests
    tests/LiveIOStatusIndicatorTests.cpp
)

target_link_libraries(my_world_live_io_status_indicator_tests
    PRIVATE
        my_world_core
)

add_test(NAME live_io_status_indicator COMMAND my_world_live_io_status_indicator_tests)

add_executable(my_world_timeline_state_tests
    tests/TimelineStateTests.cpp
)

target_link_libraries(my_world_timeline_state_tests
    PRIVATE
        my_world_storage_command
)

add_test(NAME timeline_state COMMAND my_world_timeline_state_tests)

add_executable(my_world_transport_control_tests
    tests/TransportControlTests.cpp
)

target_link_libraries(my_world_transport_control_tests
    PRIVATE
        my_world_storage_command
)

add_test(NAME transport_controls COMMAND my_world_transport_control_tests)

add_executable(my_world_variation_state_tests
    tests/VariationStateTests.cpp
)

target_link_libraries(my_world_variation_state_tests
    PRIVATE
        my_world_storage_command
)

add_test(NAME variation_state COMMAND my_world_variation_state_tests)

add_executable(my_world_parameter_row_state_tests
    tests/ParameterRowStateTests.cpp
)

target_link_libraries(my_world_parameter_row_state_tests
    PRIVATE
        my_world_storage
        my_world_interaction
)

add_test(NAME parameter_row_state COMMAND my_world_parameter_row_state_tests)

add_executable(my_world_parameter_control_tests
    tests/ParameterControlTests.cpp
)

target_link_libraries(my_world_parameter_control_tests
    PRIVATE
        my_world_storage
        my_world_interaction
)

add_test(NAME parameter_controls COMMAND my_world_parameter_control_tests)

add_executable(my_world_parameter_metadata_tests
    tests/ParameterMetadataTests.cpp
)

target_link_libraries(my_world_parameter_metadata_tests
    PRIVATE
        my_world_storage_command
)

add_test(NAME parameter_metadata COMMAND my_world_parameter_metadata_tests)

add_executable(my_world_graph_command_tests
    tests/GraphCommandTests.cpp
)

target_link_libraries(my_world_graph_command_tests
    PRIVATE
        my_world_interaction
)

add_test(NAME graph_commands COMMAND my_world_graph_command_tests)

add_executable(my_world_graph_invariant_tests
    tests/GraphInvariantTests.cpp
)

target_link_libraries(my_world_graph_invariant_tests
    PRIVATE
        my_world_interaction
)

add_test(NAME graph_invariants COMMAND my_world_graph_invariant_tests)

add_executable(my_world_node_hit_test_tests
    tests/NodeHitTestTests.cpp
)

target_link_libraries(my_world_node_hit_test_tests
    PRIVATE
        my_world_interaction
)

add_test(NAME node_hit_tests COMMAND my_world_node_hit_test_tests)

add_executable(my_world_t3_t5_command_tests
    tests/T3T5CommandTests.cpp
)

target_link_libraries(my_world_t3_t5_command_tests
    PRIVATE
        my_world_interaction
)

add_test(NAME t3_t5_commands COMMAND my_world_t3_t5_command_tests)

add_executable(my_world_interaction_storage_roundtrip_tests
    tests/InteractionStorageRoundtripTests.cpp
)

target_link_libraries(my_world_interaction_storage_roundtrip_tests
    PRIVATE
        my_world_interaction
        my_world_storage
)

add_test(NAME interaction_storage_roundtrip COMMAND my_world_interaction_storage_roundtrip_tests)

add_executable(my_world_interaction_trace_tests
    tests/InteractionTraceTests.cpp
)

target_link_libraries(my_world_interaction_trace_tests
    PRIVATE
        my_world_interaction
)

add_test(NAME interaction_traces COMMAND my_world_interaction_trace_tests)
set_tests_properties(interaction_traces PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_canvas_hands
    source/core/CanvasHands.cpp
)

target_include_directories(my_world_canvas_hands
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

target_link_libraries(my_world_canvas_hands
    PUBLIC
        my_world_interaction
)

add_executable(my_world_canvas_hands_tests
    tests/CanvasHandsTests.cpp
)

target_link_libraries(my_world_canvas_hands_tests
    PRIVATE
        my_world_canvas_hands
)

add_test(NAME canvas_hands COMMAND my_world_canvas_hands_tests)

add_executable(my_world_compound_interaction_tests
    tests/CompoundInteractionTests.cpp
)

target_link_libraries(my_world_compound_interaction_tests
    PRIVATE
        my_world_canvas_hands
        my_world_compound_module
)

add_test(NAME compound_interaction COMMAND my_world_compound_interaction_tests)
set_tests_properties(compound_interaction PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_tooll3_skin
    source/core/Tooll3SkinContract.cpp
)

target_include_directories(my_world_tooll3_skin
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

add_executable(my_world_tooll3_skin_tests
    tests/Tooll3SkinContractTests.cpp
)

target_link_libraries(my_world_tooll3_skin_tests
    PRIVATE
        my_world_tooll3_skin
)

add_test(NAME tooll3_skin_contract COMMAND my_world_tooll3_skin_tests)

add_library(my_world_audio
    source/audio/AnalyzerAggregatePressure.cpp
    source/audio/AudioAnalyzerState.cpp
    source/audio/AudioRealtimeDelivery.cpp
    source/audio/AnalyzerAttackDetector.cpp
    source/audio/AnalyzerDensityDetector.cpp
    source/audio/AnalyzerResidueDetector.cpp
    source/audio/AnalyzerSilenceDetector.cpp
    source/audio/AnalyzerSustainDetector.cpp
)

target_include_directories(my_world_audio
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/audio
)

add_executable(my_world_audio_analyzer_tests
    tests/AudioAnalyzerStateTests.cpp
)

target_link_libraries(my_world_audio_analyzer_tests
    PRIVATE
        my_world_audio
)

add_test(NAME audio_analyzer_state COMMAND my_world_audio_analyzer_tests)

add_executable(my_world_audio_realtime_delivery_tests
    tests/AudioRealtimeDeliveryTests.cpp
)

target_link_libraries(my_world_audio_realtime_delivery_tests
    PRIVATE
        my_world_audio
        my_world_core
)

add_test(NAME audio_realtime_delivery COMMAND my_world_audio_realtime_delivery_tests)

add_library(my_world_preferences
    source/preferences/PerformancePreferences.cpp
)

target_include_directories(my_world_preferences
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/preferences
)

add_executable(my_world_performance_preferences_tests
    tests/PerformancePreferencesTests.cpp
)

target_link_libraries(my_world_performance_preferences_tests
    PRIVATE
        my_world_preferences
)

add_test(NAME performance_preferences COMMAND my_world_performance_preferences_tests)

add_library(my_world_live_io_app_controller
    source/app/LiveIOAppController.cpp
)

target_include_directories(my_world_live_io_app_controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/audio
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/preferences
)

target_link_libraries(my_world_live_io_app_controller
    PUBLIC
        my_world_core
        my_world_preferences
)

add_executable(my_world_live_io_app_controller_tests
    tests/LiveIOAppControllerTests.cpp
)

target_link_libraries(my_world_live_io_app_controller_tests
    PRIVATE
        my_world_live_io_app_controller
)

add_test(NAME live_io_app_controller COMMAND my_world_live_io_app_controller_tests)

add_library(my_world_storage
    source/storage/GraphIOMappingStorage.cpp
    source/storage/StorageContract.cpp
    source/storage/StorageContractJson.cpp
    source/storage/StorageContractModules.cpp
    source/storage/StorageContractPatchDocument.cpp
    source/storage/StorageContractWorkProject.cpp
)

target_include_directories(my_world_storage
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

target_link_libraries(my_world_storage
    PUBLIC
        my_world_json
        my_world_core
)

add_library(my_world_workbench_session
    source/app/WorkbenchSession.cpp
)

target_include_directories(my_world_workbench_session
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_workbench_session
    PUBLIC
        my_world_storage
)

add_executable(my_world_workbench_session_tests
    tests/WorkbenchSessionTests.cpp
)

target_link_libraries(my_world_workbench_session_tests
    PRIVATE
        my_world_workbench_session
)

add_test(NAME workbench_session COMMAND my_world_workbench_session_tests)
set_tests_properties(workbench_session PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_work_project_lifecycle
    source/app/WorkProjectLifecycle.cpp
)

target_include_directories(my_world_work_project_lifecycle
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
)

add_executable(my_world_work_project_lifecycle_tests
    tests/WorkProjectLifecycleTests.cpp
)

target_link_libraries(my_world_work_project_lifecycle_tests
    PRIVATE
        my_world_work_project_lifecycle
)

add_test(NAME work_project_lifecycle COMMAND my_world_work_project_lifecycle_tests)

add_library(my_world_work_project_resolver
    source/app/WorkProjectResolver.cpp
)

target_include_directories(my_world_work_project_resolver
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_work_project_resolver
    PUBLIC
        my_world_proof_run_support
        my_world_storage
        my_world_work_project_lifecycle
)

add_executable(my_world_work_project_resolver_tests
    tests/WorkProjectResolverTests.cpp
)

target_link_libraries(my_world_work_project_resolver_tests
    PRIVATE
        my_world_work_project_resolver
)

add_test(NAME work_project_resolver COMMAND my_world_work_project_resolver_tests)
set_tests_properties(work_project_resolver PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_workbench_session_open_status
    source/app/WorkbenchSessionOpenStatus.cpp
)

target_include_directories(my_world_workbench_session_open_status
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_workbench_session_open_status
    PUBLIC
        my_world_proof_run_support
        my_world_work_project_resolver
        my_world_workbench_session
)

add_executable(my_world_workbench_session_open_status_tests
    tests/WorkbenchSessionOpenStatusTests.cpp
)

target_link_libraries(my_world_workbench_session_open_status_tests
    PRIVATE
        my_world_workbench_session_open_status
)

add_test(NAME workbench_session_open_status COMMAND my_world_workbench_session_open_status_tests)
set_tests_properties(workbench_session_open_status PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_workbench_app_controller
    source/app/WorkbenchAppController.cpp
)

target_include_directories(my_world_workbench_app_controller
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_workbench_app_controller
    PUBLIC
        my_world_workbench_session_open_status
        my_world_app_workbench_session_proof_runner
)

add_executable(my_world_workbench_app_controller_tests
    tests/WorkbenchAppControllerTests.cpp
)

target_link_libraries(my_world_workbench_app_controller_tests
    PRIVATE
        my_world_workbench_app_controller
)

add_test(NAME workbench_app_controller COMMAND my_world_workbench_app_controller_tests)
set_tests_properties(workbench_app_controller PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_app1_workbench_session_proof_runner
    source/app/APP1WorkbenchSessionProofRunner.cpp
)

target_include_directories(my_world_app1_workbench_session_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_app1_workbench_session_proof_runner
    PUBLIC
        my_world_proof_run_support
        my_world_workbench_session
)

add_executable(my_world_app1_workbench_session_proof_runner_tests
    tests/APP1WorkbenchSessionProofRunnerTests.cpp
)

target_link_libraries(my_world_app1_workbench_session_proof_runner_tests
    PRIVATE
        my_world_app1_workbench_session_proof_runner
)

add_test(NAME app1_workbench_session_proof_runner
         COMMAND my_world_app1_workbench_session_proof_runner_tests)
set_tests_properties(app1_workbench_session_proof_runner PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_app2_workbench_open_status_proof_runner
    source/app/APP2WorkbenchOpenStatusProofRunner.cpp
)

target_include_directories(my_world_app2_workbench_open_status_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_app2_workbench_open_status_proof_runner
    PUBLIC
        my_world_app_workbench_session_proof_runner
)

add_library(my_world_app_workbench_session_proof_runner
    source/app/AppWorkbenchSessionProofRunner.cpp
)

target_include_directories(my_world_app_workbench_session_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_app_workbench_session_proof_runner
    PUBLIC
        my_world_proof_run_support
        my_world_workbench_session_open_status
)

add_executable(my_world_app2_workbench_open_status_proof_runner_tests
    tests/APP2WorkbenchOpenStatusProofRunnerTests.cpp
)

target_link_libraries(my_world_app2_workbench_open_status_proof_runner_tests
    PRIVATE
        my_world_app2_workbench_open_status_proof_runner
)

add_test(NAME app2_workbench_open_status_proof_runner
         COMMAND my_world_app2_workbench_open_status_proof_runner_tests)
set_tests_properties(app2_workbench_open_status_proof_runner PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(my_world_app_workbench_session_proof_runner_tests
    tests/AppWorkbenchSessionProofRunnerTests.cpp
)

target_link_libraries(my_world_app_workbench_session_proof_runner_tests
    PRIVATE
        my_world_app_workbench_session_proof_runner
)

add_test(NAME app_workbench_session_proof_runner
         COMMAND my_world_app_workbench_session_proof_runner_tests)
set_tests_properties(app_workbench_session_proof_runner PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_project_creation_proof_runner
    source/app/ActiveWorkService.cpp
    source/app/AppPaths.cpp
    source/app/ProjectCreationProofRunner.cpp
)

target_include_directories(my_world_project_creation_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_project_creation_proof_runner
    PUBLIC
        my_world_proof_run_support
        my_world_work_project_resolver
        my_world_storage_command
        juce::juce_core
)

add_executable(my_world_project_creation_proof_runner_tests
    tests/ProjectCreationProofRunnerTests.cpp
)

target_link_libraries(my_world_project_creation_proof_runner_tests
    PRIVATE
        my_world_project_creation_proof_runner
)

add_test(NAME project_creation_proof_runner
         COMMAND my_world_project_creation_proof_runner_tests)

add_library(my_world_headless_render_runtime
    source/render/HeadlessRenderRuntime.cpp
)

target_include_directories(my_world_headless_render_runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/render
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

target_link_libraries(my_world_headless_render_runtime
    PRIVATE
        my_world_storage
)

add_executable(my_world_headless_render_runtime_tests
    tests/HeadlessRenderRuntimeTests.cpp
)

target_link_libraries(my_world_headless_render_runtime_tests
    PRIVATE
        my_world_headless_render_runtime
)

add_test(NAME headless_render_runtime COMMAND my_world_headless_render_runtime_tests)
set_tests_properties(headless_render_runtime PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(my_world_storage_tests
    tests/StorageContractTests.cpp
)

target_link_libraries(my_world_storage_tests
    PRIVATE
        my_world_storage
)

add_test(NAME storage_contract COMMAND my_world_storage_tests)
set_tests_properties(storage_contract PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(my_world_tixl_taxonomy_fixture_tests
    tests/TiXLTaxonomyFixtureTests.cpp
)

target_link_libraries(my_world_tixl_taxonomy_fixture_tests
    PRIVATE
        my_world_storage
)

add_test(NAME tixl_taxonomy_fixture COMMAND my_world_tixl_taxonomy_fixture_tests)
set_tests_properties(tixl_taxonomy_fixture PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(my_world_patch_document_tests
    tests/PatchDocumentTests.cpp
)

target_link_libraries(my_world_patch_document_tests
    PRIVATE
        my_world_storage
        my_world_interaction
        my_world_compound_module
)

add_test(NAME patch_document COMMAND my_world_patch_document_tests)
set_tests_properties(patch_document PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_storage_command
    source/storage/StorageCommand.cpp
)

target_include_directories(my_world_storage_command
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

target_link_libraries(my_world_storage_command
    PUBLIC
        my_world_storage
        my_world_interaction
        my_world_path_resolution
)

add_executable(my_world_save_work_command_tests
    tests/SaveWorkCommandTests.cpp
)

target_link_libraries(my_world_save_work_command_tests
    PRIVATE
        my_world_storage_command
)

add_test(NAME save_work_command COMMAND my_world_save_work_command_tests)
set_tests_properties(save_work_command PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(my_world_module_publish_tests
    tests/ModulePublishTests.cpp
)

target_link_libraries(my_world_module_publish_tests
    PRIVATE
        my_world_storage_command
        my_world_compound_module
        my_world_runtime_registry
)

add_test(NAME module_publish COMMAND my_world_module_publish_tests)
set_tests_properties(module_publish PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(my_world_analyzer_compound_family_tests
    tests/AnalyzerCompoundFamilyTests.cpp
)

target_link_libraries(my_world_analyzer_compound_family_tests
    PRIVATE
        my_world_runtime_registry
        my_world_compound_module
        my_world_interaction
)

add_test(NAME analyzer_compound_family COMMAND my_world_analyzer_compound_family_tests)
set_tests_properties(analyzer_compound_family PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_analyzer_detector_fixture
    source/core/AnalyzerAggregatePressureFixture.cpp
    source/core/AnalyzerDetectorFixture.cpp
    source/core/AnalyzerDensityDetectorFixture.cpp
    source/core/AnalyzerResidueDetectorFixture.cpp
    source/core/AnalyzerSilenceDetectorFixture.cpp
    source/core/AnalyzerSustainDetectorFixture.cpp
)

target_include_directories(my_world_analyzer_detector_fixture
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/audio
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_analyzer_detector_fixture
    PUBLIC
        my_world_runtime_registry
        my_world_storage
        my_world_json
)

add_executable(my_world_analyzer_detector_tests
    tests/AnalyzerDetectorTests.cpp
)

target_link_libraries(my_world_analyzer_detector_tests
    PRIVATE
        my_world_runtime_registry
        my_world_analyzer_detector_fixture
)

add_test(NAME analyzer_detector COMMAND my_world_analyzer_detector_tests)
set_tests_properties(analyzer_detector PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(my_world_analyzer_density_detector_tests
    tests/AnalyzerDensityDetectorTests.cpp
)

target_link_libraries(my_world_analyzer_density_detector_tests
    PRIVATE
        my_world_analyzer_detector_fixture
)

add_test(NAME analyzer_density_detector COMMAND my_world_analyzer_density_detector_tests)
set_tests_properties(analyzer_density_detector PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(my_world_analyzer_silence_detector_tests
    tests/AnalyzerSilenceDetectorTests.cpp
)

target_link_libraries(my_world_analyzer_silence_detector_tests
    PRIVATE
        my_world_analyzer_detector_fixture
)

add_test(NAME analyzer_silence_detector COMMAND my_world_analyzer_silence_detector_tests)
set_tests_properties(analyzer_silence_detector PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(my_world_analyzer_sustain_detector_tests
    tests/AnalyzerSustainDetectorTests.cpp
)

target_link_libraries(my_world_analyzer_sustain_detector_tests
    PRIVATE
        my_world_analyzer_detector_fixture
)

add_test(NAME analyzer_sustain_detector COMMAND my_world_analyzer_sustain_detector_tests)
set_tests_properties(analyzer_sustain_detector PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(my_world_analyzer_residue_detector_tests
    tests/AnalyzerResidueDetectorTests.cpp
)

target_link_libraries(my_world_analyzer_residue_detector_tests
    PRIVATE
        my_world_analyzer_detector_fixture
)

add_test(NAME analyzer_residue_detector COMMAND my_world_analyzer_residue_detector_tests)
set_tests_properties(analyzer_residue_detector PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(my_world_analyzer_aggregate_pressure_tests
    tests/AnalyzerAggregatePressureTests.cpp
)

target_link_libraries(my_world_analyzer_aggregate_pressure_tests
    PRIVATE
        my_world_analyzer_detector_fixture
)

add_test(NAME analyzer_aggregate_pressure COMMAND my_world_analyzer_aggregate_pressure_tests)
set_tests_properties(analyzer_aggregate_pressure PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_analyzer_visible_catalog
    source/core/AnalyzerVisibleCatalog.cpp
)

target_include_directories(my_world_analyzer_visible_catalog
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

target_link_libraries(my_world_analyzer_visible_catalog
    PUBLIC
        my_world_runtime_registry
        my_world_interaction
        my_world_json
)

add_executable(my_world_analyzer_visible_catalog_tests
    tests/AnalyzerVisibleCatalogTests.cpp
)

target_link_libraries(my_world_analyzer_visible_catalog_tests
    PRIVATE
        my_world_analyzer_visible_catalog
)

add_test(NAME analyzer_visible_catalog COMMAND my_world_analyzer_visible_catalog_tests)
set_tests_properties(analyzer_visible_catalog PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_proof_reports
    source/app/ProofReports.cpp
)

target_include_directories(my_world_proof_reports
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ai
        ${CMAKE_CURRENT_SOURCE_DIR}/source/audio
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_proof_reports
    PUBLIC
        my_world_json
)

add_library(my_world_c5_module_publish_proof_runner
    source/app/C5ModulePublishProofRunner.cpp
)

target_include_directories(my_world_c5_module_publish_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ai
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_c5_module_publish_proof_runner
    PUBLIC
        my_world_proof_run_support
        my_world_ai_worker
        my_world_interaction
        my_world_runtime_registry
        my_world_storage_command
        my_world_proof_reports
)

add_executable(my_world_c5_module_publish_proof_runner_tests
    tests/C5ModulePublishProofRunnerTests.cpp
)

target_link_libraries(my_world_c5_module_publish_proof_runner_tests
    PRIVATE
        my_world_c5_module_publish_proof_runner
)

add_test(NAME c5_module_publish_proof_runner
         COMMAND my_world_c5_module_publish_proof_runner_tests)
set_tests_properties(c5_module_publish_proof_runner PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_c6_analyzer_family_proof_runner
    source/app/C6AnalyzerFamilyProofRunner.cpp
)

target_include_directories(my_world_c6_analyzer_family_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

target_link_libraries(my_world_c6_analyzer_family_proof_runner
    PUBLIC
        my_world_proof_run_support
        my_world_interaction
        my_world_runtime_registry
        my_world_proof_reports
)

add_executable(my_world_c6_analyzer_family_proof_runner_tests
    tests/C6AnalyzerFamilyProofRunnerTests.cpp
)

target_link_libraries(my_world_c6_analyzer_family_proof_runner_tests
    PRIVATE
        my_world_c6_analyzer_family_proof_runner
)

add_test(NAME c6_analyzer_family_proof_runner
         COMMAND my_world_c6_analyzer_family_proof_runner_tests)
set_tests_properties(c6_analyzer_family_proof_runner PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_c6_ai_repair_loop_proof_runner
    source/app/C6AIRepairLoopProofRunner.cpp
)

target_include_directories(my_world_c6_ai_repair_loop_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ai
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_c6_ai_repair_loop_proof_runner
    PUBLIC
        my_world_proof_run_support
        my_world_ai_worker
        my_world_interaction
        my_world_proof_reports
)

add_executable(my_world_c6_ai_repair_loop_proof_runner_tests
    tests/C6AIRepairLoopProofRunnerTests.cpp
)

target_link_libraries(my_world_c6_ai_repair_loop_proof_runner_tests
    PRIVATE
        my_world_c6_ai_repair_loop_proof_runner
)

add_test(NAME c6_ai_repair_loop_proof_runner
         COMMAND my_world_c6_ai_repair_loop_proof_runner_tests)
set_tests_properties(c6_ai_repair_loop_proof_runner PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_a1_audio_proof_runner
    source/app/A1AudioProofRunner.cpp
)

target_include_directories(my_world_a1_audio_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/audio
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/preferences
)

target_link_libraries(my_world_a1_audio_proof_runner
    PUBLIC
        my_world_proof_run_support
        my_world_preferences
        my_world_runtime_registry
)

add_executable(my_world_a1_audio_proof_runner_tests
    tests/A1AudioProofRunnerTests.cpp
)

target_link_libraries(my_world_a1_audio_proof_runner_tests
    PRIVATE
        my_world_a1_audio_proof_runner
)

add_test(NAME a1_audio_proof_runner
         COMMAND my_world_a1_audio_proof_runner_tests)
set_tests_properties(a1_audio_proof_runner PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_live_io_proof_runner
    source/app/LiveIOProofRunner.cpp
)

target_include_directories(my_world_live_io_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

target_link_libraries(my_world_live_io_proof_runner
    PUBLIC
        my_world_proof_run_support
        my_world_core
        my_world_runtime_registry
)

add_executable(my_world_live_io_proof_runner_tests
    tests/LiveIOProofRunnerTests.cpp
)

target_link_libraries(my_world_live_io_proof_runner_tests
    PRIVATE
        my_world_live_io_proof_runner
)

add_test(NAME live_io_proof_runner
         COMMAND my_world_live_io_proof_runner_tests)
set_tests_properties(live_io_proof_runner PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_proof_run_support
    source/app/ProofRunSupport.cpp
)

target_include_directories(my_world_proof_run_support
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
)

add_executable(my_world_proof_run_support_tests
    tests/ProofRunSupportTests.cpp
)

target_link_libraries(my_world_proof_run_support_tests
    PRIVATE
        my_world_proof_run_support
)

add_test(NAME proof_run_support
         COMMAND my_world_proof_run_support_tests)
set_tests_properties(proof_run_support PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_g1_graph_io_mapping_proof_runner
    source/app/G1GraphIOMappingProofRunner.cpp
)

target_include_directories(my_world_g1_graph_io_mapping_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_g1_graph_io_mapping_proof_runner
    PUBLIC
        my_world_proof_run_support
        my_world_storage
)

add_executable(my_world_g1_graph_io_mapping_proof_runner_tests
    tests/G1GraphIOMappingProofRunnerTests.cpp
)

target_link_libraries(my_world_g1_graph_io_mapping_proof_runner_tests
    PRIVATE
        my_world_g1_graph_io_mapping_proof_runner
)

add_test(NAME g1_graph_io_mapping_proof_runner
         COMMAND my_world_g1_graph_io_mapping_proof_runner_tests)
set_tests_properties(g1_graph_io_mapping_proof_runner PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_c2_storage_proof_runner
    source/app/C2StorageProofRunner.cpp
)

target_include_directories(my_world_c2_storage_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_c2_storage_proof_runner
    PUBLIC
        my_world_proof_run_support
        my_world_interaction
        my_world_proof_reports
        my_world_storage
)

add_executable(my_world_c2_storage_proof_runner_tests
    tests/C2StorageProofRunnerTests.cpp
)

target_link_libraries(my_world_c2_storage_proof_runner_tests
    PRIVATE
        my_world_c2_storage_proof_runner
)

add_test(NAME c2_storage_proof_runner
         COMMAND my_world_c2_storage_proof_runner_tests)
set_tests_properties(c2_storage_proof_runner PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_c3_save_work_proof_runner
    source/app/C3SaveWorkProofRunner.cpp
)

target_include_directories(my_world_c3_save_work_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_c3_save_work_proof_runner
    PUBLIC
        my_world_proof_run_support
        my_world_interaction
        my_world_proof_reports
        my_world_storage_command
)

add_executable(my_world_c3_save_work_proof_runner_tests
    tests/C3SaveWorkProofRunnerTests.cpp
)

target_link_libraries(my_world_c3_save_work_proof_runner_tests
    PRIVATE
        my_world_c3_save_work_proof_runner
)

add_test(NAME c3_save_work_proof_runner
         COMMAND my_world_c3_save_work_proof_runner_tests)
set_tests_properties(c3_save_work_proof_runner PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_c4_ai_worker_save_work_proof_runner
    source/app/C4AIWorkerSaveWorkProofRunner.cpp
)

target_include_directories(my_world_c4_ai_worker_save_work_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ai
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_c4_ai_worker_save_work_proof_runner
    PUBLIC
        my_world_proof_run_support
        my_world_ai_worker
        my_world_interaction
        my_world_proof_reports
)

add_executable(my_world_c4_ai_worker_save_work_proof_runner_tests
    tests/C4AIWorkerSaveWorkProofRunnerTests.cpp
)

target_link_libraries(my_world_c4_ai_worker_save_work_proof_runner_tests
    PRIVATE
        my_world_c4_ai_worker_save_work_proof_runner
)

add_test(NAME c4_ai_worker_save_work_proof_runner
         COMMAND my_world_c4_ai_worker_save_work_proof_runner_tests)
set_tests_properties(c4_ai_worker_save_work_proof_runner PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_pv_b1_analyzer_environment_proof_runner
    source/app/PVB1AnalyzerEnvironmentProofRunner.cpp
)

target_include_directories(my_world_pv_b1_analyzer_environment_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

target_link_libraries(my_world_pv_b1_analyzer_environment_proof_runner
    PUBLIC
        my_world_proof_run_support
        my_world_analyzer_visible_catalog
)

add_executable(my_world_pv_b1_analyzer_environment_proof_runner_tests
    tests/PVB1AnalyzerEnvironmentProofRunnerTests.cpp
)

target_link_libraries(my_world_pv_b1_analyzer_environment_proof_runner_tests
    PRIVATE
        my_world_pv_b1_analyzer_environment_proof_runner
)

add_test(NAME pv_b1_analyzer_environment_proof_runner
         COMMAND my_world_pv_b1_analyzer_environment_proof_runner_tests)
set_tests_properties(pv_b1_analyzer_environment_proof_runner PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_pv_detector_proof_runner
    source/app/PVDetectorProofRunner.cpp
)

target_include_directories(my_world_pv_detector_proof_runner
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/app
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

target_link_libraries(my_world_pv_detector_proof_runner
    PUBLIC
        my_world_proof_run_support
        my_world_analyzer_detector_fixture
)

add_executable(my_world_pv_detector_proof_runner_tests
    tests/PVDetectorProofRunnerTests.cpp
)

target_link_libraries(my_world_pv_detector_proof_runner_tests
    PRIVATE
        my_world_pv_detector_proof_runner
)

add_test(NAME pv_detector_proof_runner COMMAND my_world_pv_detector_proof_runner_tests)
set_tests_properties(pv_detector_proof_runner PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(my_world_ai_worker
    source/ai/AIWorkerCommand.cpp
)

target_include_directories(my_world_ai_worker
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/ai
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
)

target_link_libraries(my_world_ai_worker
    PUBLIC
        my_world_storage_command
)

add_executable(my_world_ai_worker_command_tests
    tests/AIWorkerCommandTests.cpp
)

target_link_libraries(my_world_ai_worker_command_tests
    PRIVATE
        my_world_ai_worker
)

add_test(NAME ai_worker_command COMMAND my_world_ai_worker_command_tests)
set_tests_properties(ai_worker_command PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

if(EXISTS "${MY_WORLD_JUCE_DIR}/CMakeLists.txt")
    message(STATUS "Using JUCE from MY_WORLD_JUCE_DIR=${MY_WORLD_JUCE_DIR}")
    add_subdirectory("${MY_WORLD_JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE" EXCLUDE_FROM_ALL)
    set(MY_WORLD_HAVE_JUCE ON)
elseif(MY_WORLD_FETCH_JUCE)
    message(STATUS "Fetching JUCE ${MY_WORLD_JUCE_TAG} with FetchContent")
    set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG ${MY_WORLD_JUCE_TAG}
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(juce)
    set(MY_WORLD_HAVE_JUCE ON)
endif()

if(MY_WORLD_HAVE_JUCE)

    target_sources(my_world_audio
        PRIVATE
            source/audio/AudioInputAnalyzer.cpp
    )

    target_link_libraries(my_world_audio
        PUBLIC
            juce::juce_audio_devices
    )

    FetchContent_Declare(imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.8
    )

    FetchContent_MakeAvailable(imgui)

    add_library(my_world_imgui
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
        source/ui/ImGuiSmokeOverlayBrowser.cpp
        source/ui/ImGuiSmokeOverlay.cpp
        source/ui/ImGuiSmokeOverlayHelpers.cpp
        source/ui/ImGuiSmokeOverlayInspector.cpp
    )

    target_include_directories(my_world_imgui
        PUBLIC
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
            ${CMAKE_CURRENT_SOURCE_DIR}/source/ui
            ${CMAKE_CURRENT_SOURCE_DIR}/source/audio
    )

    target_link_libraries(my_world_imgui
        PUBLIC
            my_world_interaction
            my_world_node_specs
            my_world_compound
            my_world_tooll3_skin
            juce::juce_opengl
    )

    add_executable(my_world_active_work_service_tests
        source/app/ActiveWorkService.cpp
        source/app/AppPaths.cpp
        tests/ActiveWorkServiceTests.cpp
    )

    target_include_directories(my_world_active_work_service_tests
        PRIVATE
            source/app
            source/core
            source/storage
    )

    target_link_libraries(my_world_active_work_service_tests
        PRIVATE
            my_world_work_project_resolver
            my_world_storage_command
            juce::juce_core
    )

    add_test(NAME active_work_service COMMAND my_world_active_work_service_tests)
    set_tests_properties(active_work_service PROPERTIES
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    add_executable(my_world_opengl_render_backend_tests
        source/render/OpenGLRenderBackend.cpp
        tests/OpenGLRenderBackendTests.cpp
    )

    target_include_directories(my_world_opengl_render_backend_tests
        PRIVATE
            source/render
    )

    target_link_libraries(my_world_opengl_render_backend_tests
        PRIVATE
            juce::juce_opengl
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags
    )

    add_test(NAME opengl_render_backend COMMAND my_world_opengl_render_backend_tests)

    add_library(my_world_v1_shader_proof_artifacts
        source/render/V1ShaderProofArtifacts.cpp
    )

    target_include_directories(my_world_v1_shader_proof_artifacts
        PUBLIC
            source/render
            source/core
            source/audio
    )

    target_link_libraries(my_world_v1_shader_proof_artifacts
        PUBLIC
            my_world_core
            my_world_runtime_registry
            juce::juce_gui_extra
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags
    )

    add_executable(my_world_v1_shader_proof_artifacts_tests
        tests/V1ShaderProofArtifactsTests.cpp
    )

    target_link_libraries(my_world_v1_shader_proof_artifacts_tests
        PRIVATE
            my_world_v1_shader_proof_artifacts
    )

    add_test(NAME v1_shader_proof_artifacts COMMAND my_world_v1_shader_proof_artifacts_tests)
    set_tests_properties(v1_shader_proof_artifacts PROPERTIES
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    juce_add_gui_app(my-world
        PRODUCT_NAME "${MY_WORLD_DISPLAY_NAME}"
        COMPANY_NAME "Chen Po-Wei"
        BUNDLE_ID "com.chenpowei.myworld"
        MICROPHONE_PERMISSION_ENABLED TRUE
        MICROPHONE_PERMISSION_TEXT "Allow My World to analyze live audio for shader and node patches."
    )

    target_sources(my-world
        PRIVATE
            source/app/AppPaths.cpp
            source/app/ActiveWorkService.cpp
            source/app/Main.cpp
            source/app/MainComponent.cpp
            source/app/PreferencesPanel.cpp
            source/render/OpenGLRenderBackend.cpp
            source/render/OpenGLShaderPreview.cpp
    )

    target_include_directories(my-world
        PRIVATE
            source/app
            source/core
            source/ai
            source/preferences
            source/storage
            source/ui
            source/render
    )

    target_compile_definitions(my-world
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
    )

    target_link_libraries(my-world
        PRIVATE
            my_world_startup_proof
            my_world_preferences
            my_world_runtime_registry
            my_world_proof_reports
            my_world_app1_workbench_session_proof_runner
            my_world_app2_workbench_open_status_proof_runner
            my_world_workbench_app_controller
            my_world_app_workbench_session_proof_runner
            my_world_project_creation_proof_runner
            my_world_c5_module_publish_proof_runner
            my_world_c6_analyzer_family_proof_runner
            my_world_c6_ai_repair_loop_proof_runner
            my_world_a1_audio_proof_runner
            my_world_live_io_app_controller
            my_world_live_io_proof_runner
            my_world_shader_preview_input_bridge
            my_world_v1_shader_proof_artifacts
            my_world_c2_storage_proof_runner
            my_world_c3_save_work_proof_runner
            my_world_c4_ai_worker_save_work_proof_runner
            my_world_pv_b1_analyzer_environment_proof_runner
            my_world_pv_detector_proof_runner
            my_world_ai_worker
            my_world_storage_command
            my_world_imgui
            juce::juce_audio_devices
            juce::juce_audio_utils
            juce::juce_gui_extra
            juce::juce_opengl
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags
    )
else()
    message(WARNING "JUCE checkout not found. Set MY_WORLD_JUCE_DIR, JUCE_DIR, place JUCE at ../JUCE, or configure with -DMY_WORLD_FETCH_JUCE=ON. Only core tests will be configured.")
endif()
