#pragma once

#include "core/math_utilities.h"
#include "renderer/model.h"
#include "renderer/camera.h"

#include <string>
#include <memory>

namespace vre
{
	/// @brief Gives a name to the entity
	struct NameComponent
	{
		std::string name;

		NameComponent() = default;
		NameComponent(const NameComponent&) = default;
		NameComponent(const std::string& entityName)
			: name(entityName) {}
	};

	/// @brief Stores the transformation of the entity
	struct TransformComponent
	{
		glm::vec3 location = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& entityLocation)
			: location(entityLocation) {}

		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	/// @brief Holds a pointer to a model
	struct MeshComponent
	{
		std::shared_ptr<Model> model;

		MeshComponent() = default;
		MeshComponent(const MeshComponent&) = default;
		MeshComponent(std::shared_ptr<Model> entityModel)
			: model(entityModel) {}
	};

	/// @brief Creates a light 
	struct PointLightComponent
	{
		glm::vec3 color = { 1.0f, 1.0f, 1.0f };
		float intensity = 0.2f;
	};

	/// @brief Holds a camera to view the scene
	struct CameraComponent
	{
		Camera camera{};
	};

	class ScriptComponentBase;
	/// @brief Stores a custom script
	struct ScriptComponent
	{
		ScriptComponentBase* script = nullptr;
	};

} // namespace vre