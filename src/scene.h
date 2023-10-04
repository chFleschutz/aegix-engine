#pragma once

#include "device.h"
#include "scene_object.h"

namespace vre
{
	class Scene
	{
	public:
		Scene(VreDevice& device);

		VreSceneObject::Map& objects() { return mObjects; }

	protected:
		VreSceneObject::Map mObjects;
		VreDevice& mDevice;
	};

} // namespace vre
