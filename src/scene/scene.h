#pragma once

#include "graphics/device.h"
#include "graphics/model.h"
#include "graphics/renderer.h"
#include "scripting/script_manager.h"

#include <entt/entt.hpp>

#include <filesystem>


namespace Aegix::Scripting
{
	class ScriptBase;
}


namespace Aegix::Scene
{
	class Entity;

	/// @brief Base class for representation of a scene with objects
	/// @note For example subclass view DefaultScene
	/// @see default_scene.h
	class Scene
	{
	public:
		// Todo: remove device parameter
		Scene(Graphics::VulkanDevice& device, Graphics::Renderer& renderer);

		/// @brief Abstract method for creating the scene in a subclass
		virtual void initialize() = 0;

		/// @brief Creates a view of entities with components of type T
		/// @tparam ...T The components of the entities
		/// @return A view containing all entities with the given component types
		template<typename... T>
		auto viewEntitiesByType()
		{
			return m_registry.view<T...>();
		}

		// Todo: save camera somehow and allow multiple cameras
		/// @brief Returns the camera
		Entity camera();

		/// @brief Adds tracking for a script component to call its virtual functions
		void addScript(Aegix::Scripting::ScriptBase* script) { m_scriptManager.addScript(script); }

		/// @brief Calls the update function on all script components
		void update(float deltaSeconds) { m_scriptManager.update(deltaSeconds); }

		/// @brief Calls the end function on all script components
		void runtimeEnd() { m_scriptManager.runtimeEnd(); }

	protected:
		/// @brief Loads a model frome the given path
		/// @param modelPath Path to the model 
		/// @return A shared pointer with the loaded model
		/// @note The shared pointer can be used multiple times
		std::shared_ptr<Graphics::Model> loadModel(const std::filesystem::path& modelPath);

		template<typename T>
		std::shared_ptr<T> createMaterial()
		{
			//static_assert(std::is_base_of_v<Aegix::Graphics::BaseMaterial, T>, "T must derive from BaseMaterial");

			// TODO: Cyclic dependency prevents this from working
			//using SystemType = typename Graphics::RenderSystemRef<T>::type;
			//auto renderSystem = m_renderer.addRenderSystem<SystemType>();
			//return std::make_shared<T>(m_device, renderSystem->descriptorSetLayout(), m_renderer.globalPool());
			return nullptr;
		}

		/// @brief Creates an entity with a NameComponent and TransformComponent
		/// @note The entity can be passed by value since its just an id
		Entity createEntity(const std::string& name = std::string(), const Vector3& location = { 0.0f, 0.0f, 0.0f });

	private:
		Graphics::VulkanDevice& m_device;
		Graphics::Renderer& m_renderer;

		entt::registry m_registry;

		Aegix::Scripting::ScriptManager m_scriptManager;

		friend class Entity;
	};
}
