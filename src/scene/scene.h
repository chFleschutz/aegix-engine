#pragma once

#include "scene/entity.h"
#include "scripting/script_manager.h"
#include "utils/math_utils.h"

#include <entt/entt.hpp>

namespace Aegix::Scripting
{
	class ScriptBase;
}

namespace Aegix::Scene
{
	/// @brief Scene contains all entities and systems
	class Scene
	{
	public:
		Scene();
		Scene(const Scene&) = delete;
		Scene(Scene&&) = delete;
		~Scene() = default;

		auto operator=(const Scene&) -> Scene& = delete;
		auto operator=(Scene&&) -> Scene& = delete;

		/// @brief Returns the registry
		[[nodiscard]] auto registry() -> entt::registry& { return m_registry; }
		[[nodiscard]] auto mainCamera() const -> Entity { return m_mainCamera; }

		void setMainCamera(Entity camera) { m_mainCamera = camera; }

		/// @brief Adds tracking for a script component to call its virtual functions
		void addScript(Aegix::Scripting::ScriptBase* script) { m_scriptManager.addScript(script); }

		/// @brief Calls the update function on all script components
		void update(float deltaSeconds) { m_scriptManager.update(deltaSeconds); }

		/// @brief Calls the end function on all script components
		void runtimeEnd() { m_scriptManager.runtimeEnd(); }

		/// @brief Creates an entity with a NameComponent and TransformComponent
		/// @note The entity can be passed by value since its just an id
		Entity createEntity(const std::string& name = std::string(), const glm::vec3& location = { 0.0f, 0.0f, 0.0f });

		/// @brief Removes the entity from the registry
		void destroyEntity(Entity entity);

	private:
		entt::registry m_registry;
		Entity m_mainCamera;

		Aegix::Scripting::ScriptManager m_scriptManager;

		friend class Entity;
	};
}
