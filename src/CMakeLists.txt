add_library(aegix-engine STATIC)
add_library(Aegix::Engine ALIAS aegix-engine)

target_precompile_headers(aegix-engine PUBLIC pch.h)

add_subdirectory(aegix)

target_include_directories(aegix-engine PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/aegix
	${CMAKE_SOURCE_DIR}/external/stb
)

target_link_libraries(aegix-engine 
	PUBLIC 
		Vulkan::Vulkan
		EnTT::EnTT
		glfw
		glm::glm
		imgui
		imguizmo
		tinyobjloader
		GPUOpen::VulkanMemoryAllocator
		Aegix::GLTF
		Aegix::Log
)
