# Compile glsl shader files into SPIR-V

set(SHADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR})
find_program(GLSLC_EXE glslc REQUIRED)
find_program(SLANGC_EXE slangc REQUIRED)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SLANG_OPT_FLAG -O0)
    set(SLANG_DEBUG_FLAG -g)
else()
    set(SLANG_OPT_FLAG -O3)
    set(SLANG_DEBUG )
endif()

file(GLOB_RECURSE GLSL_SOURCES 
    *.glsl 
    *.vert
    *.tesc
    *.tese
    *.geom
    *.frag
    *.comp 
    *.task
    *.mesh
)

file(GLOB_RECURSE SLANG_SOURCES 
    *.slang
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

# Compile Slang shaders
foreach(SLANG ${SLANG_SOURCES})
    file(RELATIVE_PATH REL_PATH ${SHADERS_DIR} ${SLANG})
    set(SLANG_SPIRV ${CMAKE_CURRENT_BINARY_DIR}/${REL_PATH}.spv)
    list(APPEND SPIRV_FILES ${SLANG_SPIRV})

    # Ensure the output directory exists
    get_filename_component(SLANG_SPIRV_DIR ${SLANG_SPIRV} DIRECTORY)
    file(MAKE_DIRECTORY ${SLANG_SPIRV_DIR})

    add_custom_command(
        OUTPUT ${SLANG_SPIRV}
        COMMAND ${SLANGC_EXE} ${SLANG} 
            -target spirv 
            -profile spirv_1_6 
            -o ${SLANG_SPIRV} 
            -fvk-use-entrypoint-name 
            -I ${SHADERS_DIR}
            ${SLANG_OPT_FLAG} 
            ${SLANG_DEBUG_FLAG}
        DEPENDS ${SLANG}
        COMMENT "Compiling Slang Shader: ${REL_PATH}"
        VERBATIM
    )
endforeach()

add_custom_target(compile_shaders DEPENDS ${SPIRV_FILES})
add_dependencies(aegix-engine compile_shaders)
