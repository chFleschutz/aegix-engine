target_sources(aegix-engine PRIVATE
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
	"gltf_loader.cpp"
	"gltf_loader.h"
	"scene.cpp"
	"scene.h"
	"system.h"

)