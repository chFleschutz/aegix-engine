#pragma once

#include "core/asset_manager.h"
#include "graphics/systems/pbs_render_system.h"
#include "graphics/systems/point_light_system.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"

class TemplateScene : public Aegix::Scene::Scene
{
public:
	/// @brief All objects in a scene are created here
	void initialize() override
	{
		using namespace Aegix;

	}
};
