#pragma once

#include "core/asset_manager.h"
#include "core/layers/layer_stack.h"
#include "graphics/device.h"
#include "graphics/gui.h"
#include "graphics/renderer.h"
#include "graphics/window.h"
#include "scene/scene.h"

#include <memory>
#include <type_traits>

namespace Aegix
{
	class Engine
	{
	public:
		static constexpr int WIDTH = 1920;
		static constexpr int HEIGHT = 1080;
		static constexpr int MAX_FPS = 144; // Max frames per second, set 0 to disable

		Engine();
		Engine(const Engine&) = delete;
		Engine(Engine&&) = delete;
		~Engine();

		Engine& operator=(const Engine&) = delete;
		Engine& operator=(Engine&&) = delete;

		/// @brief Returns the instance of the engine
		static Engine& instance();

		Graphics::Window& window() { return m_window; }
		Graphics::Renderer& renderer() { return m_renderer; }
		AssetManager& assetManager() { return m_assetManager; }
		LayerStack& layerStack() { return m_layerStack; }
		Scene::Scene& scene() { return *m_scene; }

		void run();

		/// @brief Creates a scene of T 
		/// @tparam T Subclass of Scene which should be loaded
		/// @note T has to be a subclass of Scene otherwise compile will fail
		template<typename T>
		void loadScene()
		{
			static_assert(std::is_base_of_v<Scene::Scene, T>, "T has to be a subclass of Scene");
			m_scene = std::make_unique<T>();
		}

	private:
		void applyFrameBrake(std::chrono::steady_clock::time_point frameBeginTime);

		static Engine* s_instance;

		Graphics::Window m_window{ WIDTH, HEIGHT, "Aegix" };
		Graphics::VulkanDevice m_device{ m_window };
		Graphics::Renderer m_renderer{ m_window, m_device };
		Graphics::GUI m_gui{ m_window, m_renderer };
		
		AssetManager m_assetManager{ m_renderer };
		LayerStack m_layerStack;

		std::unique_ptr<Scene::Scene> m_scene;
	};
}
