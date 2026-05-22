cmake_minimum_required(VERSION 3.25)

project(MyWorld VERSION 0.1.0 LANGUAGES C CXX OBJC OBJCXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MY_WORLD_DISPLAY_NAME "我的世界")
set(MY_WORLD_JUCE_DIR "/Users/chenbaiwei/Documents/GitHub/sound-in-area-analyzer-plugin/JUCE" CACHE PATH "Path to a JUCE checkout")

enable_testing()

add_library(my_world_core
    source/core/GraphContract.cpp
)

target_include_directories(my_world_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/core
)

add_executable(my_world_graph_contract_tests
    tests/GraphContractTests.cpp
)

target_link_libraries(my_world_graph_contract_tests
    PRIVATE
        my_world_core
)

add_test(NAME graph_contract COMMAND my_world_graph_contract_tests)

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

add_library(my_world_audio
    source/audio/AudioAnalyzerState.cpp
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

add_library(my_world_storage
    source/storage/StorageContract.cpp
)

target_include_directories(my_world_storage
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/source/storage
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

if(EXISTS "${MY_WORLD_JUCE_DIR}/CMakeLists.txt")
    add_subdirectory("${MY_WORLD_JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE" EXCLUDE_FROM_ALL)

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
        source/ui/ImGuiSmokeOverlay.cpp
    )

    target_include_directories(my_world_imgui
        PUBLIC
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
            ${CMAKE_CURRENT_SOURCE_DIR}/source/ui
    )

    target_link_libraries(my_world_imgui
        PUBLIC
            my_world_node_specs
            juce::juce_opengl
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
            source/app/Main.cpp
            source/app/MainComponent.cpp
            source/app/PreferencesPanel.cpp
            source/render/OpenGLShaderPreview.cpp
    )

    target_include_directories(my-world
        PRIVATE
            source/app
            source/core
            source/preferences
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
            my_world_core
            my_world_audio
            my_world_preferences
            my_world_imgui
            juce::juce_audio_devices
            juce::juce_audio_utils
            juce::juce_gui_extra
            juce::juce_opengl
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags
    )
else()
    message(WARNING "JUCE checkout not found at MY_WORLD_JUCE_DIR=${MY_WORLD_JUCE_DIR}; only core tests will be configured.")
endif()
