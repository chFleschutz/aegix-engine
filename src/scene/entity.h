#pragma once

#include "scene.h"

#include <entt/entt.hpp>

#include <cassert>

namespace vre
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entityHandle, Scene* scene);


		template<typename T, typename... Args>
		T& addComponent(Args&&... args)
		{
			assert(!hasComponent<T>() && "Entity already has the component");
			return m_scene->m_registry.emplace<T>(m_entityHandle, std::forward<Args>(args)...);
		}

		template<typename... T>
		bool hasComponent()
		{
			return m_scene->m_registry.all_of<T...>(m_entityHandle);
		}

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