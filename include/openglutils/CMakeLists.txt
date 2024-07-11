cmake_minimum_required(VERSION 3.8)


project(MyImgEditor LANGUAGES CXX)





# Find the necessary packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

# Add executable
add_executable(test main.cpp)



# Include directories
target_include_directories(test PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/../  # This includes project_root/
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLFW_INCLUDE_DIRS}
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

