#pragma once

#include "scene.h"

#include <entt/entt.hpp>

#include <cassert>

namespace vre
{
	/// @brief An entity represents any object in a scene
	/// @note This class is ment to be passed by value since its just an id
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entityHandle, Scene* scene);

		/// @brief Adds a component of type T to the entity
		/// @tparam T Type of the component to add
		/// @param ...args Arguments for the constructor of T
		/// @return A refrence to the new component
		template<typename T, typename... Args>
		T& addComponent(Args&&... args)
		{
			assert(!hasComponent<T>() && "Entity already has the component");
			return m_scene->m_registry.emplace<T>(m_entityHandle, std::forward<Args>(args)...);
		}

		/// @brief Checks if the entity has a component of type T
		/// @tparam ...T Type of the component
		/// @return True if the entity has a component of type T otherwise false
		template<typename... T>
		bool hasComponent()
		{
			return m_scene->m_registry.all_of<T...>(m_entityHandle);
		}

		/// @brief Acces to the component of type T
		/// @tparam T Type of the component
		/// @return A reference to the component 
		template<typename T>
		T& getComponent()
		{
			assert(hasComponent<T>() && "Entity does not have the component");
			return m_scene->m_registry.get<T>(m_entityHandle);
		}

	private:
		entt::entity m_entityHandle = { entt::null };
		Scene* m_scene = nullptr;
	};

} // namespace vre