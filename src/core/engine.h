#pragma once

#include "core/asset_manager.h"
#include "core/input.h"
#include "core/systems/system_manager.h"
#include "graphics/device.h"
#include "graphics/globals.h"
#include "graphics/renderer.h"
#include "graphics/window.h"
#include "scene/scene.h"
#include "ui/ui.h"

#include <memory>
#include <type_traits>

namespace Aegix
{
	class Engine
	{
	public:
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
		Graphics::VulkanDevice& device() { return m_device; }
		Graphics::Renderer& renderer() { return m_renderer; }
		Graphics::UI& ui() { return m_ui; }
		AssetManager& assetManager() { return m_assetManager; }
		SystemManager& systemManager() { return m_systems; }
		Scene::Scene& scene() { assert(m_scene);  return *m_scene; }

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

		inline static Engine* s_instance = nullptr;

		Graphics::Window m_window{ Graphics::DEFAULT_WIDTH, Graphics::DEFAULT_HEIGHT, "Aegix" };
		Graphics::VulkanDevice m_device{ m_window };
		Graphics::Renderer m_renderer{ m_window, m_device };
		Graphics::UI m_ui{ m_window, m_renderer };
		Input m_input{ m_window };
		AssetManager m_assetManager{ m_renderer };
		SystemManager m_systems;

		std::unique_ptr<Scene::Scene> m_scene;
	};
}
