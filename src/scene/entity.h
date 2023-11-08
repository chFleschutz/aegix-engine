#pragma once

#include "scene/scene.h"
#include "scene/components.h"

#include <entt/entt.hpp>

#include <cassert>

namespace VEScene
{
	/// @brief An entity represents any object in a scene
	/// @note This class is ment to be passed by value since its just an id
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entityHandle, Scene* scene)
			: m_entityHandle{ entityHandle }, m_scene{ scene } {}

		/// @brief Adds a component of type T to the entity
		/// @tparam T Type of the component to add
		/// @param ...args Arguments for the constructor of T
		/// @return A refrence to the new component
		/// @note When adding a custom script ScriptComponent is added as its container
		template<typename T, typename... Args>
		T& addComponent(Args&&... args)
		{
			assert(!hasComponent<T>() && "Entity already has the component");
			return m_scene->m_registry.emplace<T>(m_entityHandle, std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		VEComponent::Script& addScript(Args&&... args)
		{
			assert(!hasComponent<T>() && "Entity already has the component");
			auto& script = m_scene->m_registry.emplace<VEComponent::Script>(m_entityHandle);
			script.script = std::make_unique<T>(std::forward<Args>(args)...);
			return script;
		}

		/// @brief Checks if the entity has all components of type T...
		/// @tparam ...T Type of the components to check
		/// @return True if the entity has all components of type T... otherwise false
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

} // namespace VEScene