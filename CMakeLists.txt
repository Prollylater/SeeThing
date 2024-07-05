cmake_minimum_required(VERSION 3.8)


project(MyImgEditor LANGUAGES CXX)


add_executable(MyImgEditor TP1.cpp)


target_include_directories(MyImgEditor PUBLIC include)

# Optionally, if you have additional source files, you can add them like this:
# target_sources(MyProject PRIVATE src/AnotherSource.cpp)

# Add libraries if necessary (for example, if you need to link to an external library)
# target_link_libraries(MyProject PRIVATE some_library)

