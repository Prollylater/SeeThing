#TODO Refine CmakeList

cmake_minimum_required(VERSION 3.8)
project(MyImgEditor LANGUAGES CXX)

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)


# Project directories
set(SOURCE_DIR  src)
set(INCLUDE_DIR includes)


# altrnative file(GLOB PROJECT_SRC "*.cpp")
set(PROJECT_SRC
    #Core
    ${SOURCE_DIR}/main.cpp
    
    ${SOURCE_DIR}/Core/ImgApp.cpp
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
    
    ${SOURCE_DIR}/Utils/Imloader.cpp
    ${SOURCE_DIR}/Utils/Mat.cpp
   
   #Imgui
    ${INCLUDE_DIR}/Imgui/imgui.cpp
    ${INCLUDE_DIR}/Imgui/backends/imgui_impl_glfw.cpp    
    ${INCLUDE_DIR}/Imgui/backends/imgui_impl_opengl3.cpp
    ${INCLUDE_DIR}/Imgui/imgui_demo.cpp
    ${INCLUDE_DIR}/Imgui/imgui_draw.cpp
    ${INCLUDE_DIR}/Imgui/imgui_tables.cpp
    ${INCLUDE_DIR}/Imgui/imgui_widgets.cpp


)


# altrnative file(GLOB INCLUDES_DIRS "*.h")
set(INCLUDES_DIRS 
   
    ${INCLUDE_DIR}
    ${INCLUDE_DIR}/3rdParty
    
    ${INCLUDE_DIR}/ImgEditorEngine
    #Core
    ${INCLUDE_DIR}/Core
    #Openglutils
    ${INCLUDE_DIR}/Openglutils
    #RenderContext
    ${INCLUDE_DIR}/RenderContext
    #UiWindows
    ${INCLUDE_DIR}/UiWindows
    #Utils
    ${INCLUDE_DIR}/Utils
   
   
    #Imgui
    ${INCLUDE_DIR}/Imgui/
    ${INCLUDE_DIR}/Imgui/backends
    

)




file(COPY ressources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


# Add executable
add_executable(test ${PROJECT_SRC})


# Include directories
target_include_directories(test PUBLIC 
    ${INCLUDES_DIRS}   
)


# Link libraries
target_link_libraries(test
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    glfw
)


# Optionally, if you have additional source files, you can add them like this:
# target_sources(MyProject PRIVATE src/AnotherSource.cpp)

# Add libraries if necessary (for example, if you need to link to an external library)
# target_link_libraries(MyProject PRIVATE some_library)

