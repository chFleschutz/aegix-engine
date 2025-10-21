add_library(aegix-engine STATIC)
add_library(Aegix::Engine ALIAS aegix-engine)

target_precompile_headers(aegix-engine PUBLIC pch.h)

add_subdirectory(aegix)

target_include_directories(aegix-engine PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/aegix
)

target_link_libraries(aegix-engine 
	PUBLIC 
		Aegix::GLTF
		Aegix::Log
		EnTT::EnTT
		glfw
		glm::glm
		imgui
		imguizmo
		meshoptimizer
		stb
		tinyobjloader
		GPUOpen::VulkanMemoryAllocator
		volk::volk_headers
)
