
add_library(AegixEngine STATIC)

target_sources(AegixEngine PRIVATE
	"core/asset_manager.cpp"
	"core/engine.cpp"
	"core/input.cpp"
	"core/layers/layer_stack.cpp"
	"graphics/buffer.cpp"
	"graphics/camera.cpp"
	"graphics/descriptors.cpp"
	"graphics/device.cpp"
	"graphics/model.cpp"
	"graphics/pipeline.cpp"
	"graphics/renderer.cpp"
	"graphics/swap_chain.cpp"
	"graphics/texture.cpp"
	"graphics/window.cpp"
	"graphics/systems/default_render_system.cpp"
	"graphics/systems/point_light_system.cpp"
	"graphics/systems/render_system.cpp"
	"physics/motion_dynamics.cpp"
	"scene/entity.cpp"
	"scene/scene.cpp"
	"scripting/script_manager.cpp"
	"scripting/movement/dynamic_movement_controller.cpp"
	"scripting/movement/kinematic_movement_controller.cpp"
	"utils/color.cpp"
	"utils/math_utils.cpp"
	"utils/perlin_noise.cpp"
	"utils/random.cpp"
)

target_include_directories(AegixEngine PUBLIC 
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${CMAKE_SOURCE_DIR}/lib/entt/single_include"
	"${CMAKE_SOURCE_DIR}/lib/glm"
	"${CMAKE_SOURCE_DIR}/lib/stb"
	"${CMAKE_SOURCE_DIR}/lib/tinyobjloader"
)

target_link_libraries(AegixEngine PUBLIC 
	Vulkan::Vulkan
	glfw
	imgui
)
