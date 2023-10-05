#pragma once

#include "device.h"
#include "scene_entity.h"

#include <filesystem>

namespace vre
{
	/// @brief Base class for representation of a scene with objects
	/// @note For example subclass view DefaultScene
	/// @see default_scene.h
	class Scene
	{
	public:
		Scene(VreDevice& device);

		/// @brief Access to the map containing all objects of the scene
		/// @return Returns a reference to the map  
		SceneEntity::Map& entities() { return mObjects; }

		SceneEntity& camera() { return mCamera; }

	protected:
		/// @brief Creates an empty SceneEntity
		/// @param location World location of the entity
		/// @param rotation Absolute rotation of the entity
		/// @param scale Absolute scale of the Enity
		/// @return Returns a reference to the created entity
		SceneEntity& createEntity(const glm::vec3& location = { 0.0f, 0.0f, 0.0f }, const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f }, const glm::vec3& scale = { 1.0f, 1.0f, 1.0f });
		
		/// @brief Creates a SceneEntity with a model
		/// @param model 3D-Model which should be displayed
		/// @param location World location of the object
		/// @return Returns a reference to the created SceneEntity
		SceneEntity& createEntity(std::shared_ptr<VreModel> model, const glm::vec3& location = { 0.0f, 0.0f, 0.0f }, const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f }, const glm::vec3& scale = { 1.0f, 1.0f, 1.0f });
		
		/// @brief Creates a Pointlight
		/// @param intensity Intensity of the light
		/// @param location World location of the light
		/// @return Returns a reference to the created SceneEntity
		SceneEntity& createPointLight(const glm::vec3& location = { 0.0f, 0.0f, 0.0f }, float intensity = 1.0f, float radius = 0.1f, const glm::vec3& color = { 1.0f, 1.0f, 1.0f });
	
		std::shared_ptr<VreModel> loadModel(const std::filesystem::path& modelPath);

	private:
		VreDevice& mDevice;
		SceneEntity::Map mObjects;
		SceneEntity& mCamera;
	};

} // namespace vre
