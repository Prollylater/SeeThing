# Yet to be Named OpenGL-Based Application

This application offer a graphical interface and code base aiming to be modulable for testing and implementing computer vision algorithm going from segmentation to color transfer and much more.
The project is an OpenGL application. It uses ImGui for the graphical user interface, glad to load OpenGL and CMake to build.

## Table of Contents
1. [Features](#features)
2. [Prerequisites](#prerequisites)
3. [Building and Running the Project](#building-and-running-the-project)
4. [Configuration](#configuration)
5. [Usage](#usage)

## Features currently implemented 
- ImGui-based graphical user friendly interface (Should be soon!)
- Importing image (and soon mesh) and running them through numerous vision algorithm either visual or informative.
- Lot of parameters adjustments for conversion, modification and fine-tuning results.

## Prerequisites

Before building and running the application, make sure you have the following software installed:
- **CMake** 3.16 or higher
- A **C++20** compiler (GCC, Clang or MSVC)
- **git** (CMake downloads GLFW and Dear ImGui with it on the first configure)
- A GPU and driver with **OpenGL 4.5**
- **OpenMP** (optional, for multithreaded SLIC)

CMake downloads and builds GLFW 3.5.1 and Dear ImGui 1.93.0 (docking). The OpenGL loader (glad) and two ImGui add-ons are in `third_party/`.

## Building and Running the Project

To build the project, follow these steps:

1. **Clone the repository:**
   ```bash
   git clone https://github.com/Prollylater/SeeThing.git
   cd SeeThing
   ```
2. **Install the build tools.** On Linux, GLFW also needs the X11 and Wayland headers. The command below is for Debian and Ubuntu. On other systems, use your package manager. Windows needs only Visual Studio, CMake and git.
   ```bash
   sudo apt install build-essential cmake git libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
                    libwayland-dev libxkbcommon-dev
   ```
3. **Build and run:**
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build -j
   ./build/seething
   ```
   Every build copies `ressources/` next to the executable, so it runs from any folder.
   To load shaders and fonts from somewhere else, set `SEETHING_ASSET_DIR`, for example
   `SEETHING_ASSET_DIR=$PWD/ressources ./build/seething`.
   The first configure downloads GLFW and ImGui into the build folder. Set `CPM_SOURCE_CACHE=$HOME/.cache/CPM` to share
   that download between build folders and to configure offline afterwards.
4. **Run the tests:**
   ```bash
   ctest --test-dir build --output-on-failure
   ```
   The tests cover the core image, colour and file code. To skip them and the doctest download, configure with `-DSEETHING_BUILD_TESTS=OFF`.

![image](https://github.com/user-attachments/assets/33926deb-2afa-4857-ac03-36b3907d4e49)
