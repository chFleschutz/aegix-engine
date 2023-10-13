#pragma once

#include "renderer/descriptors.h"
#include "renderer/device.h"
#include "scene/scene.h"
#include "renderer/window.h"
#include "renderer/renderer.h"

#include <memory>
#include <type_traits>
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

		/// @brief Creates a scene of T 
		/// @tparam T Subclass of Scene which should be loaded
		/// @note T has to be a subclass of Scene otherwise compile will fail
		template<class T, class = std::enable_if_t<std::is_base_of_v<Scene, T>>>
		void loadScene()
		{
			mScene = std::make_unique<T>(mVreDevice);
			mScene->initialize();
		}

	private:
		VreWindow mVreWindow{ WIDTH, HEIGHT, "Vulkanite" };
		VreDevice mVreDevice{ mVreWindow };
		Renderer mVreRenderer{ mVreWindow, mVreDevice };

		std::unique_ptr<VreDescriptorPool> mGlobalPool{};
		std::unique_ptr<Scene> mScene;
	};

} // namespace vre
