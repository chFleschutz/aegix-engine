#pragma once

#include "device.h"
#include "scene_object.h"

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
		VreSceneObject::Map& objects() { return mObjects; }

	protected:
		/// @brief Creates a VreSceneObject with a model
		/// @param model 3D-Model which should be displayed
		/// @param location World location of the object
		/// @return Returns a reference to the created VreSceneObject
		VreSceneObject& create(std::shared_ptr<VreModel> model, const glm::vec3& location = { 0.0f, 0.0f, 0.0f });
		
		/// @brief Creates a Pointlight
		/// @param intensity Intensity of the light
		/// @param location World location of the light
		/// @return Returns a reference to the created VreSceneObject
		VreSceneObject& createPointLight(float intensity, const glm::vec3& location = { 0.0f, 0.0f, 0.0f });
	
		std::shared_ptr<VreModel> loadModel(const std::filesystem::path& modelPath);

	private:
		VreSceneObject::Map mObjects;
		VreDevice& mDevice;
	};

} // namespace vre
