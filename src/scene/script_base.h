#pragma once

#include "scene/entity.h"
#include "scene/components.h"

namespace VEScripting
{
	class ScriptBase
	{
	public:
		virtual ~ScriptBase() {}

	protected:
		/// @brief Called once at the beginning of the scene
		virtual void begin() {}
		/// @brief Called every frame with the delta of the last two frames in seconds
		virtual void update(float deltaSeconds) {}
		/// @brief Called once at the end of the scene
		virtual void end() {}

		/// @brief Returns true if the entity has all components of type T...
		template<typename... T>
		bool hasComponent()
		{
			return m_entity->hasComponent<T...>();
		}

		/// @brief Returns a reference of the component of type T
		/// @note Component of type T must exist
		template<typename T>
		T& getComponent()
		{
			return m_entity.getComponent<T>();
		}

		/// @brief Adds a component of type T to the entity and returns a reference to it
		template<typename T, typename... Args>
		T& addComponent(Args&&... args)
		{
			return m_entity.addComponent<T>(std::forward<Args>(args)...);
		}

		/// @brief Adds a script of type T to the entity and returns a reference to it
		template<typename T, typename... Args>
		VEComponent::Script& addScript(Args&&... args)
		{
			return m_entity.addScript<T>(std::forward<Args>(args)...);
		}

	private:
		VEScene::Entity m_entity;

		friend class VEScene::Scene;
	};

} // namespace vre
