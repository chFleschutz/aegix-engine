add_library(aegix-engine STATIC)
add_library(Aegix::Engine ALIAS aegix-engine)

target_include_directories(aegix-engine 
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}
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
		Aegix::GLTF
		Aegix::Log
)

add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(physics)
add_subdirectory(scene)
add_subdirectory(scripting)
add_subdirectory(ui)
add_subdirectory(utils)
