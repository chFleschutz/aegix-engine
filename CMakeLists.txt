cmake_minimum_required(VERSION 3.28)

project(AegixEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


option(BUILD_EXAMPLES "Build example projects" ON)
option(COMPILE_SHADERS "Compile GLSL shaders to SPIR-V" ON)


# Find the Vulkan package
find_package(Vulkan COMPONENTS glslc REQUIRED)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan was not found. Make sure the Vulkan SDK is installed")
endif()
if(NOT Vulkan_glslc_FOUND)
    message(FATAL_ERROR "glslc was not found. Make sure glslc is included in the Vulkan SDK")
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


add_definitions(-DPROJECT_DIR="${CMAKE_SOURCE_DIR}")

add_subdirectory("lib/aegix-gltf")

set(GLFW_BUILD_DOCS OFF)
add_subdirectory("lib/glfw")

add_subdirectory("lib/imgui")

add_subdirectory("src")

if(BUILD_EXAMPLES)
    add_subdirectory("examples/simple-scene")
    add_subdirectory("examples/pbs-scene")
endif()


# Compile glsl shader files into SPIR-V
if(COMPILE_SHADERS)
    add_custom_target(compile_shaders)

    set(SHADERS_DIR "${CMAKE_SOURCE_DIR}/shaders")
    file(GLOB_RECURSE SHADER_SOURCES 
        "${SHADERS_DIR}/*.glsl" 
        "${SHADERS_DIR}/*.vert"
        "${SHADERS_DIR}/*.tesc"
        "${SHADERS_DIR}/*.tese"
        "${SHADERS_DIR}/*.geom"
        "${SHADERS_DIR}/*.frag"
        "${SHADERS_DIR}/*.comp" 
    )

    foreach(GLSL ${SHADER_SOURCES})
        get_filename_component(SHADER_NAME ${GLSL} NAME)
        set(SPIRV "${SHADERS_DIR}/${SHADER_NAME}.spv")

        add_custom_command(
            TARGET compile_shaders
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL}
            COMMENT "Compiling Shader: ${GLSL} to ${SPIRV}"
            VERBATIM
        )
    endforeach()

    add_dependencies(${PROJECT_NAME} compile_shaders)
endif()
