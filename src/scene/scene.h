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
	/// @brief Base class for representation of a scene with objects
	/// @note For example subclass view DefaultScene
	/// @see default_scene.h
	class Scene
	{
	public:
		Scene();
		virtual ~Scene() = default;

		/// @brief Abstract method for creating the scene in a subclass
		virtual void initialize() = 0;

		/// @brief Creates a view of entities with components of type T
		/// @tparam ...T The components of the entities
		/// @return A view containing all entities with the given component types
		template<typename... T>
		auto viewEntities()
		{
			return m_registry.view<T...>();
		}

		/// @brief Returns the registry
		entt::registry& registry() { return m_registry; }

		// Todo: save camera somehow and allow multiple cameras
		/// @brief Returns the camera
		Entity camera();

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

		Aegix::Scripting::ScriptManager m_scriptManager;

		friend class Entity;
	};
}
