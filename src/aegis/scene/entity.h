#pragma once

#include "scene/component_traits.h"

#include <entt/entt.hpp>

namespace Aegis::Scripting
{
	class ScriptBase;

	template<typename T>
	concept Script = std::is_base_of_v<ScriptBase, T>;
}

namespace Aegis::Scene
{
	class Scene;

	template<typename T>
	concept TagComponent = std::is_empty_v<T>;

	/// @brief An entity represents any object in a scene
	/// @note This class is ment to be passed by value since its just an id
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entityHandle, Scene* scene);
		Entity(const Entity&) = default;
		Entity(Entity&&) = default;

		auto operator=(const Entity&) -> Entity& = default;
		auto operator=(Entity&&) -> Entity& = default;

		bool operator==(const Entity& other) const;
		bool operator!=(const Entity& other) const;
		
		operator bool() const { return m_id != entt::null; }
		operator entt::entity() const { return m_id; }
		operator uint32_t() const { return static_cast<uint32_t>(m_id); }

		[[nodiscard]] auto registry() const -> entt::registry&;

		/// @brief Checks if the entity has all components of type T...
		template<typename... T>
		auto has() const -> bool
		{
			return registry().all_of<T...>(m_id);
		}

		/// @brief Acces to the component of type T
		template<typename T>
		auto get() const -> T&
		{
			AGX_ASSERT_X(has<T>(), "Cannot get Component: Entity does not have the component");
			return registry().get<T>(m_id);
		}

		/// @brief Adds a component of type T to the entity
		/// @return A refrence to the new component
		template<typename T, typename... Args>
		auto add(Args&&... args) -> T&
		{
			AGX_ASSERT_X(!has<T>(), "Cannot add Component: Entity already has the component");
			return registry().emplace<T>(m_id, std::forward<Args>(args)...);
		}

		/// @brief Overload to add tag components (empty structs) to the entity
		template<TagComponent T>
		auto add()
		{
			AGX_ASSERT_X(!has<T>(), "Cannot add Component: Entity already has the component");
			registry().emplace<T>(m_id);
		}

		/// @brief Overload to add a script derived from Aegis::Scripting::ScriptBase to the entity
		/// @return A refrence to the new script
		template<Scripting::Script T, typename... Args>
		auto add(Args&&... args) -> T&
		{
			AGX_ASSERT_X(!has<T>(), "Cannot add Component: Entity already has the component");
			auto& script = registry().emplace<T>(m_id, std::forward<Args>(args)...);
			addScript(&script);
			return script;
		}

		template<typename T>
		auto getOrAdd() -> T&
		{
			return registry().get_or_emplace<T>(m_id);
		}

		/// @brief Removes a component of type T from the entity
		/// @note Entity MUST have the component and it MUST be an optional component
		template<typename T>
			requires IsOptionalComponent<T>
		void remove()
		{
			AGX_ASSERT_X(has<T>(), "Cannot remove Component: Entity does not have the component");
			registry().remove<T>(m_id);
		}

		void setParent(Entity parent);
		void removeParent();
		void addChild(Entity child);
		void removeChild(Entity child);
		void removeChildren();

	private:
		void addScript(Scripting::ScriptBase* script);

		entt::entity m_id{ entt::null };
		Scene* m_scene{ nullptr };
	};
}
