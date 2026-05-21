cmake_minimum_required(VERSION 3.25)

project(MyWorld VERSION 0.1.0 LANGUAGES C CXX OBJC OBJCXX)

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

if(EXISTS "${MY_WORLD_JUCE_DIR}/CMakeLists.txt")
    add_subdirectory("${MY_WORLD_JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE" EXCLUDE_FROM_ALL)

    juce_add_gui_app(my-world
        PRODUCT_NAME "${MY_WORLD_DISPLAY_NAME}"
        COMPANY_NAME "Chen Po-Wei"
        BUNDLE_ID "com.chenpowei.myworld"
    )

    target_sources(my-world
        PRIVATE
            source/app/Main.cpp
            source/app/MainComponent.cpp
            source/render/OpenGLShaderPreview.cpp
    )

    target_include_directories(my-world
        PRIVATE
            source/app
            source/core
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
            juce::juce_gui_extra
            juce::juce_opengl
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags
    )
else()
    message(WARNING "JUCE checkout not found at MY_WORLD_JUCE_DIR=${MY_WORLD_JUCE_DIR}; only core tests will be configured.")
endif()
