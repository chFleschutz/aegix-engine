#pragma once

#include "scene/entity.h"
#include "scene/system.h"
#include "scripting/script_manager.h"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace Aegix::Scripting
{
	class ScriptBase;
}

namespace Aegix::Scene
{
	/// @brief Scene contains all entities and systems
	class Scene
	{
		friend class Entity;

	public:
		Scene();
		Scene(const Scene&) = delete;
		Scene(Scene&&) = delete;
		~Scene() = default;

		auto operator=(const Scene&) -> Scene& = delete;
		auto operator=(Scene&&) -> Scene& = delete;

		[[nodiscard]] auto registry() -> entt::registry& { return m_registry; }
		[[nodiscard]] auto mainCamera() const -> Entity { return m_mainCamera; }

		void setMainCamera(Entity camera) { m_mainCamera = camera; }

		template <SystemDerived T, typename... Args>
			requires std::constructible_from<T, Args...>
		void addSystem(Args&&... args)
		{
			m_systems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
		}

		/// @brief Creates an entity with a NameComponent and TransformComponent
		/// @note Scene::Entity can be passed by value
		Entity createEntity(const std::string& name = std::string(), const glm::vec3& location = { 0.0f, 0.0f, 0.0f });

		void destroyEntity(Entity entity);

		/// @brief Adds tracking for a script component to call its virtual functions
		void addScript(Scripting::ScriptBase* script) { m_scriptManager.addScript(script); }

		void update(float deltaSeconds);

	private:
		entt::registry m_registry;
		std::vector<std::unique_ptr<System>> m_systems;
		Scripting::ScriptManager m_scriptManager;
		Entity m_mainCamera;
	};
}
