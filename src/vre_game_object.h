#pragma once

#include "vre_model.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace vre
{
	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{1.0f, 1.0f, 1.0f};
		glm::vec3 rotation{};

		// Transformation-matrix: Translate * Ry * Rx * Rz * Scale
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};

	struct PointLightComponent
	{
		float lightIntensity = 1.0f;
	};

	class VreGameObject
	{
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, VreGameObject>;

		static VreGameObject createGameObject()
		{
			static id_t currentId = 0;
			return VreGameObject{ currentId++ };
		}

		static VreGameObject makePointLight(float intensity = 10.0f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.0f));

		VreGameObject(const VreGameObject&) = delete;
		VreGameObject& operator=(const VreGameObject&) = delete;
		VreGameObject(VreGameObject&&) = default;
		VreGameObject& operator=(VreGameObject&&) = default;

		id_t id() { return mId; }

		glm::vec3 color{};
		TransformComponent transform{};

		// Optional components
		std::shared_ptr<VreModel> model{};
		std::unique_ptr<PointLightComponent> pointLight = nullptr;

	private:
		VreGameObject(id_t objId) : mId{ objId } {}

		id_t mId;
	};

} // namespace vre


