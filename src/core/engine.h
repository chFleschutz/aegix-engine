#pragma once

#include "graphics/descriptors.h"
#include "graphics/device.h"
#include "graphics/renderer.h"
#include "graphics/window.h"
#include "scene/scene.h"

#include <memory>
#include <type_traits>
#include <vector>


namespace Aegix
{
	class Engine
	{
	public:
		static constexpr int WIDTH = 1080;
		static constexpr int HEIGHT = 720;
		static constexpr int MAX_FPS = 144; // Max frames per second, set 0 to disable

		Engine();
		~Engine() = default;

		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		void run();

		/// @brief Creates a scene of T 
		/// @tparam T Subclass of Scene which should be loaded
		/// @note T has to be a subclass of Scene otherwise compile will fail
		template<typename T>
		void loadScene()
		{
			static_assert(std::is_base_of_v<Scene::Scene, T>, "T has to be a subclass of Scene");
			m_scene = std::make_unique<T>(m_device);
		}

	private:
		void applyFrameBrake(std::chrono::steady_clock::time_point frameBeginTime);

		Graphics::Window m_window{ WIDTH, HEIGHT, "Aegix" };
		Graphics::VulkanDevice m_device{ m_window };
		Graphics::Renderer m_renderer{ m_window, m_device };

		std::unique_ptr<Scene::Scene> m_scene;
	};
}
