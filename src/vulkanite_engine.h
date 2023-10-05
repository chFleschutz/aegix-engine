#pragma once

#include "descriptors.h"
#include "device.h"
#include "scene_entity.h"
#include "window.h"
#include "renderer.h"
#include "scene.h"

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
		}


	private:
		void loadSceneOld();

		VreWindow mVreWindow{ WIDTH, HEIGHT, "Vulkanite" };
		VreDevice mVreDevice{ mVreWindow };
		VreRenderer mVreRenderer{ mVreWindow, mVreDevice };

		std::unique_ptr<VreDescriptorPool> mGlobalPool{};
		SceneEntity::Map mGameObjects;
		std::unique_ptr<Scene> mScene;
	};

} // namespace vre
