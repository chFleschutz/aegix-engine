#pragma once

#include "scene/entity.h"
#include "scene/components.h"

namespace Aegix::Scripting
{
	class ScriptBase
	{
	public:
		/// @brief Constructor
		/// @note When overriding, dont't use member functions (m_entity is not initialized yet)
		ScriptBase() = default;
		virtual ~ScriptBase() = default;

		/// @brief Called once at the first frame just before update
		virtual void begin() {}
		/// @brief Called every frame with the delta of the last two frames in seconds
		virtual void update(float deltaSeconds) {}
		/// @brief Called once at the end of the last frame
		virtual void end() {}

		/// @brief Returns true if the entity has all components of type T...
		template<typename... T>
		bool hasComponent() const
		{
			return m_entity.hasComponent<T...>();
		}

		/// @brief Returns a reference of the component of type T
		/// @note Component of type T must exist
		template<typename T>
		T& component() const
		{
			return m_entity.component<T>();
		}

		/// @brief Adds a component of type T to the entity and returns a reference to it
		template<typename T, typename... Args>
		T& addComponent(Args&&... args)
		{
			return m_entity.addComponent<T>(std::forward<Args>(args)...);
		}

		/// @brief Returns the entity
		Aegix::Scene::Entity entity() const { return m_entity; }

	private:
		Aegix::Scene::Entity m_entity;

		friend class Aegix::Scene::Entity;
	};
}
