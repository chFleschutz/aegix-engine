cmake_minimum_required(VERSION 3.28)

project(AegixEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_EXAMPLES "Build example projects" ON)
option(COMPILE_SHADERS "Compile GLSL shaders to SPIR-V" ON)

# Find the Vulkan package
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan was not found. Make sure the Vulkan SDK is installed")
endif()
if(NOT Vulkan_glslc_FOUND)
    message(FATAL_ERROR "glslc was not found. Make sure glslc is included in the Vulkan SDK")
endif()

add_definitions(-DPROJECT_DIR="${CMAKE_SOURCE_DIR}")
add_definitions(-DBUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}")
add_subdirectory(external)
add_subdirectory(modules)
add_subdirectory(src)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(COMPILE_SHADERS)
    add_subdirectory(shaders)
endif()

# Print configuration
message("")
message("CMake Configuration:")
message(STATUS "CMake Version: ${CMAKE_VERSION}")
message(STATUS "CMake Generator: ${CMAKE_GENERATOR}")
message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMake source directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMake binary directory: ${CMAKE_BINARY_DIR}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Vulkan SDK ${Vulkan_VERSION}: ${Vulkan_LIBRARY}")
message(STATUS "GLSL Compiler: ${Vulkan_GLSLC_EXECUTABLE}")
message("")
