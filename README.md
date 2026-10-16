# Yet to be Named OpenGL-Based Application

This application offer a graphical interface and code base aiming to be modulable for testing and implementing computer vision algorithm going from segmentation to color transfer and much more.
The project is an OpenGL-based application utilizing ImGui for the graphical user interface, GLEW for managing OpenGL extensions, and CMake for building the project.

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
- A **C++17** compiler (GCC, Clang or MSVC)
- **GLEW**
- **GLFW** 3
- **OpenGL** (version 3.3 or higher)
- **OpenMP** (optional: SLIC uses it when found)

## Building and Running the Project

To build the project, follow these steps:

1. **Clone the repository:**
   ```bash
   git clone https://github.com/Prollylater/SeeThing.git
   cd SeeThing
   ```
2. **Install the dependencies** (Debian/Ubuntu shown; use your package manager otherwise):
   ```bash
   sudo apt install build-essential cmake libglew-dev libglfw3-dev
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

![image](https://github.com/user-attachments/assets/33926deb-2afa-4857-ac03-36b3907d4e49)
