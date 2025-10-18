# Compile glsl shader files into SPIR-V

find_program(GLSLC_EXE glslc REQUIRED)

set(SHADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE GLSL_SOURCES 
    *.glsl 
    *.vert
    *.tesc
    *.tese
    *.geom
    *.frag
    *.comp 
)

set(SPIRV_FILES "")

# Compile GLSL shaders
foreach(GLSL ${GLSL_SOURCES})
    file(RELATIVE_PATH REL_PATH ${SHADERS_DIR} ${GLSL})
    set(SPIRV ${CMAKE_CURRENT_BINARY_DIR}/${REL_PATH}.spv)
    list(APPEND SPIRV_FILES ${SPIRV})

    # Ensure the output directory exists
    get_filename_component(SPIRV_DIR ${SPIRV} DIRECTORY)
    file(MAKE_DIRECTORY ${SPIRV_DIR})

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSLC_EXE} --target-env=vulkan1.3 -o ${SPIRV} ${GLSL} 
        DEPENDS ${GLSL}
        COMMENT "Compiling GLSL Shader: ${REL_PATH}"
        VERBATIM
    )
endforeach()

add_custom_target(compile_shaders DEPENDS ${SPIRV_FILES})
add_dependencies(aegix-engine compile_shaders)
