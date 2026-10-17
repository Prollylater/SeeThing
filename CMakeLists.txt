cmake_minimum_required(VERSION 3.16)
project(SeeThing LANGUAGES C CXX)

include(cmake/CPM.cmake)
CPMAddPackage(NAME glfw3 GITHUB_REPOSITORY glfw/glfw GIT_TAG 3.5.1
              OPTIONS "GLFW_BUILD_DOCS OFF" "GLFW_BUILD_TESTS OFF" "GLFW_BUILD_EXAMPLES OFF" "GLFW_INSTALL OFF")
# Read docs/CHANGELOG.txt of the new tag before moving it
CPMAddPackage(NAME imgui GITHUB_REPOSITORY ocornut/imgui GIT_TAG v1.93.0-docking DOWNLOAD_ONLY YES)
find_package(OpenMP)

option(SEETHING_BUILD_TESTS "Build the unit tests (downloads doctest)" ON)


# Project directories
set(SOURCE_DIR  src)
set(INCLUDE_DIR includes)


# Dear ImGui, built apart so the project's warning flags don't apply to it
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui SYSTEM PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    third_party/imgui_addons
)
target_compile_definitions(imgui PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)
target_link_libraries(imgui PUBLIC glfw ${CMAKE_DL_LIBS})


# GL 4.5 core loader from glad 2.0.8; the command that regenerates it is at the top of gl.h
add_library(glad STATIC third_party/glad/src/gl.c)
target_include_directories(glad SYSTEM PUBLIC third_party/glad/include)


add_library(seething_warnings INTERFACE)
target_compile_options(seething_warnings INTERFACE
    "$<$<CXX_COMPILER_ID:MSVC>:/W4>"
    "$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall;-Wextra>"
)


# Core: images, colour, I/O. No GL, no UI; the tests link only this
add_library(seething_core STATIC
    ${SOURCE_DIR}/core/Color.cpp
    ${SOURCE_DIR}/core/ImageIO.cpp
    ${SOURCE_DIR}/core/Log.cpp
)
target_compile_features(seething_core PUBLIC cxx_std_20)
target_include_directories(seething_core PUBLIC ${SOURCE_DIR})
target_include_directories(seething_core SYSTEM PRIVATE ${INCLUDE_DIR}/3rdParty)
target_link_libraries(seething_core PRIVATE seething_warnings)


# Document: layers, selection, history. CPU only
add_library(seething_doc STATIC
    ${SOURCE_DIR}/doc/Document.cpp
    ${SOURCE_DIR}/doc/Edits.cpp
    ${SOURCE_DIR}/doc/History.cpp
)
target_link_libraries(seething_doc PUBLIC seething_core PRIVATE seething_warnings)


add_library(seething_gpu STATIC
    ${SOURCE_DIR}/gpu/DebugOutput.cpp
    ${SOURCE_DIR}/gpu/Framebuffer.cpp
    ${SOURCE_DIR}/gpu/Program.cpp
    ${SOURCE_DIR}/gpu/Renderer.cpp
    ${SOURCE_DIR}/gpu/ShaderCache.cpp
    ${SOURCE_DIR}/gpu/Texture.cpp
)
target_link_libraries(seething_gpu PUBLIC seething_core glad PRIVATE seething_warnings)


set(PROJECT_SRC
    ${SOURCE_DIR}/app/main.cpp
    ${SOURCE_DIR}/app/App.cpp
    ${SOURCE_DIR}/app/Platform.cpp
    ${SOURCE_DIR}/ui/Shell.cpp
    ${SOURCE_DIR}/ui/CanvasWidget.cpp
    ${SOURCE_DIR}/ui/SelectionOverlay.cpp
    ${SOURCE_DIR}/ui/panels/CanvasPanel.cpp
    ${SOURCE_DIR}/ui/panels/HistoryPanel.cpp
    ${SOURCE_DIR}/ui/panels/LayersPanel.cpp
    ${SOURCE_DIR}/ui/panels/LogPanel.cpp
    ${SOURCE_DIR}/ui/panels/ResultPanel.cpp
    ${SOURCE_DIR}/ui/panels/SegmentationPanel.cpp
    ${SOURCE_DIR}/ImgEditorEngine/ImgEditorEngine.cpp
    ${SOURCE_DIR}/ImgEditorEngine/RegionGrowingAlg.cpp
    ${SOURCE_DIR}/ImgEditorEngine/Regions.cpp
    ${SOURCE_DIR}/Utils/AssetPath.cpp
    ${SOURCE_DIR}/Utils/Mat.cpp
    ${SOURCE_DIR}/Utils/Helper.cpp
)

set(INCLUDES_DIRS
    ${INCLUDE_DIR}
    ${INCLUDE_DIR}/ImgEditorEngine
    ${INCLUDE_DIR}/Utils
)


add_executable(seething ${PROJECT_SRC})
target_compile_features(seething PRIVATE cxx_std_20)
target_include_directories(seething PRIVATE ${INCLUDES_DIRS})
target_include_directories(seething SYSTEM PRIVATE ${INCLUDE_DIR}/3rdParty)
target_link_libraries(seething PRIVATE seething_core seething_doc seething_gpu imgui glad glfw seething_warnings)

if(OpenMP_CXX_FOUND)
    target_link_libraries(seething PRIVATE OpenMP::OpenMP_CXX)
    message(STATUS "OpenMP found and linked to the project.")
else()
    message(STATUS "OpenMP not found: SLIC runs on one thread.")
endif()


# Runs on every build, so edited shaders and fonts are picked up without re-running CMake
add_custom_target(seething_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/ressources $<TARGET_FILE_DIR:seething>/ressources
    COMMENT "Copying ressources/ next to the executable"
)


if(SEETHING_BUILD_TESTS)
    CPMAddPackage(NAME doctest GITHUB_REPOSITORY doctest/doctest GIT_TAG v2.5.3
                  OPTIONS "DOCTEST_NO_INSTALL ON" "DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF")
    enable_testing()
    add_executable(seething_tests
        tests/core/test_main.cpp
        tests/core/test_image.cpp
        tests/core/test_color.cpp
        tests/core/test_imageio.cpp
        tests/ui/test_canvas_view.cpp
        tests/doc/test_document.cpp
    )
    target_link_libraries(seething_tests PRIVATE seething_core seething_doc doctest::doctest seething_warnings)
    add_test(NAME core COMMAND seething_tests)

    # Headless GL 4.5 through EGL; skipped (exit 77) where no context can be made
    add_executable(seething_gpu_tests
        tests/gpu/test_main.cpp
        tests/gpu/test_gpu.cpp
    )
    target_compile_definitions(seething_gpu_tests PRIVATE SEETHING_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/ressources/shaders")
    target_link_libraries(seething_gpu_tests PRIVATE seething_gpu glfw doctest::doctest seething_warnings)
    add_test(NAME gpu COMMAND seething_gpu_tests)
    set_tests_properties(gpu PROPERTIES SKIP_RETURN_CODE 77)
endif()
