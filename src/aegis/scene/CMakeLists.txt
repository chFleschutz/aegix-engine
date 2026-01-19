target_sources(aegis-engine PRIVATE
	"loader/fast_gltf_loader.h"
	"loader/fast_gltf_loader.cpp"
	"loader/gltf_loader.cpp"
	"loader/gltf_loader.h"
	"loader/obj_loader.cpp"
	"loader/obj_loader.h"

	"systems/camera_system.cpp"
	"systems/camera_system.h"
	"systems/transform_system.cpp"
	"systems/transform_system.h"
	
	"components.h"
	"description.h"
	"entity.cpp"
	"entity.h"
	"scene.cpp"
	"scene.h"
	"system.h"

)