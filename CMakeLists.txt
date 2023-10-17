cmake_minimum_required(VERSION 3.12)

set(NAME Vulkanite)
project(${NAME})

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the Vulkan package
find_package(Vulkan COMPONENTS glslc REQUIRED)
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan was not found. Make sure the Vulkan SDK is installed")
endif()
if (NOT Vulkan_glslc_FOUND)
    message(FATAL_ERROR "glslc was not found")
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


# Compile glsl shader files into SPIR-V
# Get shader files
set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/shaders)
file(GLOB_RECURSE SHADER_SOURCES 
    "${SHADERS_DIR}/*.frag" 
    "${SHADERS_DIR}/*.vert"
)
add_custom_target(compile_shaders)

# Compile each shader
foreach(GLSL ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${GLSL} NAME)
    set(SPIRV ${SHADERS_DIR}/${SHADER_NAME}.spv)
    add_custom_command(
        TARGET compile_shaders
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
        VERBATIM
    )
endforeach()
add_dependencies(${PROJECT_NAME} compile_shaders)


# Print configuration
message("CMake Configuration:")
message(STATUS "CMake Version: ${CMAKE_VERSION}")
message(STATUS "CMake Generator: ${CMAKE_GENERATOR}")
message(STATUS "CMake Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMake source directory: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMake binary directory: ${CMAKE_BINARY_DIR}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Vulkan SDK ${Vulkan_VERSION}: ${Vulkan_LIBRARY}")
message(STATUS "GLSL Compiler: ${Vulkan_GLSLC_EXECUTABLE}")
