#pragma once

#include "vre_model.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace vre
{
	struct TransformComponent
	{
		glm::vec3 location{};
		glm::vec3 rotation{};
		glm::vec3 scale{1.0f, 1.0f, 1.0f};

		// Transformation-matrix: Translate * Ry * Rx * Rz * Scale
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	struct PointLightComponent
	{
		float lightIntensity = 1.0f;
	};

	class VreSceneObject
	{
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, VreSceneObject>;

		static VreSceneObject createEmpty();
		static VreSceneObject createPointLight(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.0f));
		static VreSceneObject createModel(std::shared_ptr<VreModel> model);

		VreSceneObject(const VreSceneObject&) = delete;
		VreSceneObject& operator=(const VreSceneObject&) = delete;
		VreSceneObject(VreSceneObject&&) = default;
		VreSceneObject& operator=(VreSceneObject&&) = default;

		id_t id() { return mId; }

		glm::vec3 color{};
		TransformComponent transform{};

		// Optional components
		std::shared_ptr<VreModel> model{};
		std::unique_ptr<PointLightComponent> pointLight = nullptr;

	private:
		VreSceneObject(id_t objId) : mId{ objId } {}

		id_t mId;
	};

} // namespace vre


