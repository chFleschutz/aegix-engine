#pragma once

#include <entt/entt.hpp>

#include <cassert>

namespace Aegix::Scripting
{
	class ScriptBase;

	template<typename T>
	concept Script = std::is_base_of_v<ScriptBase, T>;
}

namespace Aegix::Scene
{
	class Scene;

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

		[[nodiscard]] auto registry() const -> entt::registry&;

		/// @brief Checks if the entity has all components of type T...
		template<typename... T>
		auto hasComponent() const -> bool
		{
			return registry().all_of<T...>(m_entityID);
		}

		/// @brief Acces to the component of type T
		template<typename T>
		auto component() const -> T&
		{
			assert(hasComponent<T>() && "Cannot get Component: Entity does not have the component");
			return registry().get<T>(m_entityID);
		}

		/// @brief Adds a component of type T to the entity
		/// @return A refrence to the new component
		template<typename T, typename... Args>
		auto addComponent(Args&&... args) -> T&
		{
			assert(!hasComponent<T>() && "Cannot add Component: Entity already has the component");
			return registry().emplace<T>(m_entityID, std::forward<Args>(args)...);
		}

		/// @brief Overload to add a script derived from Aegix::Scripting::ScriptBase to the entity
		/// @return A refrence to the new script
		template<Scripting::Script T, typename... Args>
		auto addComponent(Args&&... args) -> T&
		{
			assert(!hasComponent<T>() && "Cannot add Component: Entity already has the component");
			auto& script = registry().emplace<T>(m_entityID, std::forward<Args>(args)...);
			addScript(&script);
			return script;
		}

		template<typename T>
		auto getOrAddComponent() -> T&
		{
			return registry().get_or_emplace<T>(m_entityID);
		}

		/// @brief Removes a component of type T from the entity
		template<typename T>
		void removeComponent()
		{
			assert(hasComponent<T>() && "Cannot remove Component: Entity does not have the component");
			registry().remove<T>(m_entityID);
		}

		/// @brief Sets the parent of the entity
		void setParent(Entity parent);

		/// @brief Adds a child to the entity
		void addChild(Entity child);

		/// @brief Removes a child from the entity
		/// @note Make sure that 'child' is an actual child of this entity
		void removeChild(Entity child);

	private:
		void addScript(Scripting::ScriptBase* script);

		entt::entity m_entityID = { entt::null };
		Scene* m_scene = nullptr;
	};
}
