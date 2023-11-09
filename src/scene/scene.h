#pragma once

#include "graphics/device.h"
#include "graphics/model.h"

#include <entt/entt.hpp>

#include <filesystem>

namespace VEScripting
{
	class ScriptBase;
}

namespace VEScene
{
	class Entity;

	/// @brief Base class for representation of a scene with objects
	/// @note For example subclass view DefaultScene
	/// @see default_scene.h
	class Scene
	{
	public:
		// Todo: remove device parameter
		Scene(VEGraphics::VulkanDevice& device);

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

		/// @brief Initialized script components and calls the begin() function
		void runtimeBegin();
		/// @brief Calls the update function on all script components
		void update(float deltaSeconds);
		/// @brief Calls the end function on all script components
		void runtimeEnd();

		void addScript(VEScripting::ScriptBase* script);

	protected:
		/// @brief Loads a model frome the given path
		/// @param modelPath Path to the model 
		/// @return A shared pointer with the loaded model
		/// @note The shared pointer can be used multiple times
		std::shared_ptr<VEGraphics::Model> loadModel(const std::filesystem::path& modelPath);

		/// @brief Creates an entity with a NameComponent and TransformComponent
		/// @note The entity can be passed by value since its just an id
		Entity createEntity(const std::string& name = std::string(), const Vector3& location = { 0.0f, 0.0f, 0.0f });

	private:
		VEGraphics::VulkanDevice& m_device;
		entt::registry m_registry;

		std::vector<VEScripting::ScriptBase*> m_scripts;

		friend class Entity;
	};

} // namespace VEScene
