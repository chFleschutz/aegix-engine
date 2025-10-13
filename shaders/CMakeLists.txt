# Compile glsl shader files into SPIR-V

set(SHADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB_RECURSE SHADER_SOURCES 
    *.glsl 
    *.vert
    *.tesc
    *.tese
    *.geom
    *.frag
    *.comp 
)

set(SPIRV_FILES "")
foreach(GLSL ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${GLSL} NAME)
    set(SPIRV ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.spv)
    list(APPEND SPIRV_FILES ${SPIRV})

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
        COMMENT "Compiling Shader: ${GLSL}"
        VERBATIM
    )
endforeach()

add_custom_target(
    compile_shaders
    DEPENDS ${SPIRV_FILES}
)

add_dependencies(aegix-engine compile_shaders)
