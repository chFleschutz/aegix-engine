target_sources(aegix-engine PRIVATE
	"editor_layer.cpp"
	"editor_layer.h"
	"input.cpp"
	"input.h"
	"layer.h"
	"layer_stack.h"
	"logging.h"
	"profiler.h"
	"window.cpp"
	"window.h"
  "../graphics/render_passes/transparent_pass.cpp" "asset.h" "asset_manager.h" "asset_manager.cpp" "../graphics/render_systems/point_light_render_system.h" "../graphics/render_systems/static_mesh_render_system.cpp" "../graphics/render_systems/point_light_render_system.cpp")