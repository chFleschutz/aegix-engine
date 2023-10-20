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
			m_scene = std::make_unique<T>(m_device);
			m_scene->initialize();
		}

	private:
		Window m_window{ WIDTH, HEIGHT, "Vulkanite" };
		VulkanDevice m_device{ m_window };
		Renderer m_renderer{ m_window, m_device };

		std::unique_ptr<DescriptorPool> m_globalPool{};
		std::unique_ptr<Scene> m_scene;
	};

} // namespace vre
