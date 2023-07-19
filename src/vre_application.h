#pragma once

#include "vre_descriptors.h"
#include "vre_device.h"
#include "vre_scene_object.h"
#include "vre_window.h"
#include "vre_renderer.h"

#include <memory>
#include <vector>

namespace vre
{
	class VreApplication
	{
	public:
		static constexpr int WIDTH = 1080;
		static constexpr int HEIGHT = 720;

		VreApplication();
		~VreApplication();

		VreApplication(const VreApplication&) = delete;
		VreApplication& operator=(const VreApplication&) = delete;

		void run();

	private:
		void loadScene();

		VreWindow mVreWindow{ WIDTH, HEIGHT, "Vulkanite" };
		VreDevice mVreDevice{ mVreWindow };
		VreRenderer mVreRenderer{ mVreWindow, mVreDevice };

		std::unique_ptr<VreDescriptorPool> mGlobalPool{};
		VreSceneObject::Map mGameObjects;
	};

} // namespace vre
