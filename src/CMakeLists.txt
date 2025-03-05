add_library(aegix-engine STATIC)
add_library(Aegix::Engine ALIAS aegix-engine)

target_include_directories(aegix-engine 
	PUBLIC 
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${CMAKE_SOURCE_DIR}/external/entt/single_include"
		"${CMAKE_SOURCE_DIR}/external/stb"
		"${CMAKE_SOURCE_DIR}/external/tinyobjloader"
)

target_link_libraries(aegix-engine 
	PUBLIC 
		Vulkan::Vulkan
		glfw
		glm::glm
		imgui
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
