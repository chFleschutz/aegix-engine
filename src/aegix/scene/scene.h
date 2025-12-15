#pragma once

#include "scene/entity.h"
#include "scene/system.h"
#include "scripting/script_manager.h"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace Aegix::Scripting
{
	class ScriptBase;
}

namespace Aegix::Scene
{
	/// @brief Scene contains all entities and systems
	class Scene
	{
		friend class Entity;

	public:
		Scene() = default;
		Scene(const Scene&) = delete;
		Scene(Scene&&) = delete;
		~Scene() = default;

		auto operator=(const Scene&) -> Scene& = delete;
		auto operator=(Scene&&) -> Scene& = delete;

		[[nodiscard]] auto registry() -> entt::registry& { return m_registry; }
		[[nodiscard]] auto mainCamera() const -> Entity { return m_mainCamera; }
		[[nodiscard]] auto ambientLight() const -> Entity { return m_ambientLight; }
		[[nodiscard]] auto directionalLight() const -> Entity { return m_directionalLight; }
		[[nodiscard]] auto environment() const -> Entity { return m_skybox; }

		void setMainCamera(Entity camera) { m_mainCamera = camera; }

		template <SystemDerived T, typename... Args>
			requires std::constructible_from<T, Args...>
		void addSystem(Args&&... args)
		{
			m_systems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
		}

		/// @brief Creates an entity with a NameComponent and TransformComponent
		/// @note Scene::Entity can be passed by value
		auto createEntity(const std::string& name = std::string(), const glm::vec3& location = glm::vec3{ 0.0f },
			const glm::vec3& rotation = glm::vec3{ 0.0f }, const glm::vec3& scale = glm::vec3{ 1.0f }) -> Entity;

		void destroyEntity(Entity entity);

		/// @brief Adds tracking for a script component to call its virtual functions
		void addScript(Scripting::ScriptBase* script) { m_scriptManager.addScript(script); }

		void begin();

		void update(float deltaSeconds);

		/// @brief Loads a scene from a file and returns the root entity
		auto load(const std::filesystem::path& path) -> Entity;

		void reset();

	private:
		entt::registry m_registry;
		std::vector<std::unique_ptr<System>> m_systems;
		Scripting::ScriptManager m_scriptManager;

		Entity m_mainCamera;
		Entity m_ambientLight;
		Entity m_directionalLight;
		Entity m_skybox;
	};
}
