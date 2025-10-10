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
  "../graphics/render_passes/transparent_pass.cpp")