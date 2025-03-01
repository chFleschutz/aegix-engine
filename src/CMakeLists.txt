add_library(aegix-engine STATIC)
add_library(Aegix::Engine ALIAS aegix-engine)

target_include_directories(aegix-engine 
	PUBLIC 
		"${CMAKE_CURRENT_SOURCE_DIR}"
		"${CMAKE_SOURCE_DIR}/lib/entt/single_include"
		"${CMAKE_SOURCE_DIR}/lib/glm"
		"${CMAKE_SOURCE_DIR}/lib/stb"
		"${CMAKE_SOURCE_DIR}/lib/tinyobjloader"
)

target_link_libraries(aegix-engine 
	PUBLIC 
		Vulkan::Vulkan
		glfw
		imgui
		Aegix::GLTF
)

add_subdirectory(core)
add_subdirectory(graphics)
add_subdirectory(physics)
add_subdirectory(scene)
add_subdirectory(scripting)
add_subdirectory(ui)
add_subdirectory(utils)
