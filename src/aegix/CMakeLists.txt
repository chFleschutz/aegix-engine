add_library(aegix-engine STATIC)
add_library(Aegix::Engine ALIAS aegix-engine)

target_include_directories(aegix-engine 
PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/..
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_include_directories(aegix-engine 
PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/..
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

target_precompile_headers(aegix-engine PUBLIC pch.h)

add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(math)
add_subdirectory(physics)
add_subdirectory(scene)
add_subdirectory(scripting)
add_subdirectory(ui)
add_subdirectory(utils)

target_sources(aegix-engine PRIVATE
	engine.cpp
	engine.h
)