cmake_minimum_required(VERSION 3.16)
project(SeeThing LANGUAGES C CXX)

include(cmake/CPM.cmake)
CPMAddPackage(NAME glfw3 GITHUB_REPOSITORY glfw/glfw GIT_TAG 3.5.1
              OPTIONS "GLFW_BUILD_DOCS OFF" "GLFW_BUILD_TESTS OFF" "GLFW_BUILD_EXAMPLES OFF" "GLFW_INSTALL OFF")
# Read docs/CHANGELOG.txt of the new tag before moving it
CPMAddPackage(NAME imgui GITHUB_REPOSITORY ocornut/imgui GIT_TAG v1.93.0-docking DOWNLOAD_ONLY YES)
find_package(OpenMP)


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


set(PROJECT_SRC
    #Core
    ${SOURCE_DIR}/main.cpp
    ${SOURCE_DIR}/Core/ImgApp.cpp
    #Models
    ${SOURCE_DIR}/Models/Canvas.cpp
    #ImgEditorEngine
    ${SOURCE_DIR}/ImgEditorEngine/ImgEditorEngine.cpp
    ${SOURCE_DIR}/ImgEditorEngine/RegionGrowingAlg.cpp
    ${SOURCE_DIR}/ImgEditorEngine/Regions.cpp
    #Openglutils
    ${SOURCE_DIR}/Openglutils/Buffers.cpp
    ${SOURCE_DIR}/Openglutils/Program.cpp
    ${SOURCE_DIR}/Openglutils/Texture.cpp
    #RenderContext
    ${SOURCE_DIR}/RenderContext/ImGuiAppContext.cpp
    ${SOURCE_DIR}/RenderContext/OpenGLContext.cpp
    #UiWindows
    ${SOURCE_DIR}/UiWindows/Dockspace.cpp
    ${SOURCE_DIR}/UiWindows/ImgEditor.cpp
    ${SOURCE_DIR}/UiWindows/RGPanel.cpp
    ${SOURCE_DIR}/UiWindows/UiCompUtils.cpp
    ${SOURCE_DIR}/UiWindows/GlobalStates.cpp
    #Utils
    ${SOURCE_DIR}/Utils/AssetPath.cpp
    ${SOURCE_DIR}/Utils/Imloader.cpp
    ${SOURCE_DIR}/Utils/Mat.cpp
    ${SOURCE_DIR}/Utils/Helper.cpp
)

set(INCLUDES_DIRS
    ${INCLUDE_DIR}
    ${INCLUDE_DIR}/Models
    ${INCLUDE_DIR}/ImgEditorEngine
    ${INCLUDE_DIR}/Core
    ${INCLUDE_DIR}/Openglutils
    ${INCLUDE_DIR}/RenderContext
    ${INCLUDE_DIR}/UiWindows
    ${INCLUDE_DIR}/Utils
)


add_executable(seething ${PROJECT_SRC})
target_compile_features(seething PRIVATE cxx_std_20)
target_include_directories(seething PRIVATE ${INCLUDES_DIRS})
target_include_directories(seething SYSTEM PRIVATE ${INCLUDE_DIR}/3rdParty)
target_link_libraries(seething PRIVATE imgui glad glfw seething_warnings)

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
