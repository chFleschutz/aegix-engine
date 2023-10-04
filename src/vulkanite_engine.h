#pragma once

#include "descriptors.h"
#include "device.h"
#include "scene_object.h"
#include "window.h"
#include "renderer.h"

#include <memory>
#include <vector>

namespace vre
{
	class VulkaniteEngine
	{
	public:
		static constexpr int WIDTH = 1080;
		static constexpr int HEIGHT = 720;

		VulkaniteEngine();
		~VulkaniteEngine();

		VulkaniteEngine(const VulkaniteEngine&) = delete;
		VulkaniteEngine& operator=(const VulkaniteEngine&) = delete;

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
