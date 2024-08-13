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
		Entity(entt::entity entityHandle, Scene* scene)
			: m_entityHandle{ entityHandle }, m_scene{ scene } {}

		bool operator==(const Entity& other) const
		{
			return m_entityHandle == other.m_entityHandle && m_scene == other.m_scene;
		}

		/// @brief Checks if the entity has all components of type T...
		/// @tparam ...T Type of the components to check
		/// @return True if the entity has all components of type T... otherwise false
		template<typename... T>
		bool hasComponent() const
		{
			return m_scene->m_registry.all_of<T...>(m_entityHandle);
		}

		/// @brief Acces to the component of type T
		/// @tparam T Type of the component
		/// @return A reference to the component 
		template<typename T>
		T& getComponent() const
		{
			assert(hasComponent<T>() && "Entity does not have the component");
			return m_scene->m_registry.get<T>(m_entityHandle);
		}

		/// @brief Adds a component of type T to the entity
		/// @tparam T Type of the component to add
		/// @param ...args Arguments for the constructor of T
		/// @return A refrence to the new component
		/// @note When adding a custom script ScriptComponent is added as its container
		template<typename T, typename... Args>
		typename std::enable_if_t<!std::is_base_of_v<Scripting::ScriptBase, T>, T&>
			addComponent(Args&&... args)
		{
			assert(!hasComponent<T>() && "Entity already has the component");
			return m_scene->m_registry.emplace<T>(m_entityHandle, std::forward<Args>(args)...);
		}

		/// @brief Adds a script derived from Aegix::Scripting::ScriptBase to the entity
		template<typename T, typename... Args>
		typename std::enable_if_t<std::is_base_of_v<Scripting::ScriptBase, T>, T&>
			addComponent(Args&&... args)
		{
			assert(!hasComponent<T>() && "Entity already has the component");
			auto& script = m_scene->m_registry.emplace<T>(m_entityHandle, std::forward<Args>(args)...);
			script.m_entity = *this;
			m_scene->addScript(&script);
			return script;
		}

	private:
		entt::entity m_entityHandle = { entt::null };
		Scene* m_scene = nullptr;
	};
}
