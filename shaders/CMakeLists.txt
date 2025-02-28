# Compile glsl shader files into SPIR-V
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

add_dependencies(aegix-engine compile_shaders)
