
add_library(AegixEngine STATIC)

target_sources(AegixEngine PRIVATE
	"core/asset_manager.cpp"
	"core/engine.cpp"
	"core/input.cpp"
	"core/systems/camera_system.cpp" 
	"core/systems/system_manager.cpp"

	"graphics/buffer.cpp"
	"graphics/descriptors.cpp"
	"graphics/device.cpp"
	"graphics/deletion_queue.h"
	"graphics/deletion_queue.cpp"
	"graphics/frame_graph/frame_graph.cpp"
	"graphics/frame_graph/frame_graph_blackboard.h"
	"graphics/frame_graph/frame_graph_render_pass.h"
	"graphics/frame_graph/frame_graph_resource_pool.h"
	"graphics/frame_graph/frame_graph_resource_pool.cpp"
	"graphics/frame_graph/render_stage.h"
	"graphics/globals.h"
	"graphics/render_passes/geometry_pass.h" 
	"graphics/render_passes/gui_pass.h" 
	"graphics/render_passes/lighting_pass.h"
	"graphics/render_passes/post_processing_pass.h"
	"graphics/render_passes/present_pass.h"
	"graphics/render_passes/transparent_pass.h"
	
	"graphics/gui.cpp"
	"graphics/layers/scene_properties_layer.cpp" 
	"graphics/pipeline.cpp"
	"graphics/renderer.cpp"
	"graphics/static_mesh.cpp"
	"graphics/swap_chain.cpp"
	"graphics/systems/default_render_system.cpp"
	"graphics/systems/point_light_system.cpp"
	"graphics/systems/pbs_render_system.cpp"
	"graphics/systems/render_system.cpp"
	"graphics/texture.cpp"
	"graphics/vulkan_tools.cpp"
	"graphics/window.cpp"

	"physics/motion_dynamics.cpp"

	"scene/entity.cpp"
	"scene/scene.cpp"

	"scripting/script_manager.cpp"
	"scripting/movement/dynamic_movement_controller.cpp"
	"scripting/movement/kinematic_movement_controller.cpp"

	"utils/color.cpp"
	"utils/file.cpp"
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
	Aegix::GLTF
)
