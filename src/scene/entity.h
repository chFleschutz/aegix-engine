#pragma once

#include "scene/scene.h"

#include <entt/entt.hpp>

#include <cassert>

namespace Aegix::Scene
{
	/// @brief An entity represents any object in a scene
	/// @note This class is ment to be passed by value since its just an id
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entityHandle, Scene* scene);

		bool operator==(const Entity& other) const;
		bool operator!=(const Entity& other) const;
		
		operator bool() const { return m_entityID != entt::null; }
		operator entt::entity() const { return m_entityID; }
		operator uint32_t() const { return static_cast<uint32_t>(m_entityID); }

		/// @brief Checks if the entity has all components of type T...
		template<typename... T>
		bool hasComponent() const
		{
			return m_scene->m_registry.all_of<T...>(m_entityID);
		}

		/// @brief Acces to the component of type T
		template<typename T>
		T& getComponent() const
		{
			assert(hasComponent<T>() && "Cannot get Component: Entity does not have the component");
			return m_scene->m_registry.get<T>(m_entityID);
		}

		/// @brief Adds a component of type T to the entity
		/// @return A refrence to the new component
		template<typename T, typename... Args>
		typename std::enable_if_t<!std::is_base_of_v<Scripting::ScriptBase, T>, T&>
			addComponent(Args&&... args)
		{
			assert(!hasComponent<T>() && "Cannot add Component: Entity already has the component");
			return m_scene->m_registry.emplace<T>(m_entityID, std::forward<Args>(args)...);
		}

		/// @brief Overload to add a script derived from Aegix::Scripting::ScriptBase to the entity
		/// @return A refrence to the new script
		template<typename T, typename... Args>
		typename std::enable_if_t<std::is_base_of_v<Scripting::ScriptBase, T>, T&>
			addComponent(Args&&... args)
		{
			assert(!hasComponent<T>() && "Cannot add Component: Entity already has the component");
			auto& script = m_scene->m_registry.emplace<T>(m_entityID, std::forward<Args>(args)...);
			script.m_entity = *this;
			m_scene->addScript(&script);
			return script;
		}

		/// @brief Removes a component of type T from the entity
		template<typename T>
		void removeComponent()
		{
			assert(hasComponent<T>() && "Cannot remove Component: Entity does not have the component");
			m_scene->m_registry.remove<T>(m_entityID);
		}

	private:
		entt::entity m_entityID = { entt::null };
		Scene* m_scene = nullptr;
	};
}
