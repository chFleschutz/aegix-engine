cmake_minimum_required(VERSION 3.12)

set(NAME Vulkanite)
project(${NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("CMake Configuration:")
message(STATUS "CMake Version: ${CMAKE_VERSION}")
message(STATUS "CMake Generator: ${CMAKE_GENERATOR}")
message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMake source directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMake binary directory: ${CMAKE_BINARY_DIR}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")

# Find the Vulkan package
find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    message(STATUS "Vulkan SDK found")
else()
    message(FATAL_ERROR "Vulkan SDK was not found")
endif()

# Top-level source directory 
include_directories(src)

# External libraries
include_directories(
    lib/entt/single_include
    lib/glfw/include
    lib/glm
    lib/stb
    lib/tinyobjloader
)

# Recursively find all .cpp files in src
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link glfw
target_link_libraries(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/lib/glfw/lib-vc2022/glfw3.lib)
# Link vulkan
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
