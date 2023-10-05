#include "scene_entity.h"

namespace vre
{
	glm::mat4 Transform::mat4()
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		return glm::mat4{
			{
				scale.x* (c1* c3 + s1 * s2 * s3),
				scale.x* (c2* s3),
				scale.x* (c1* s2* s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{
				location.x, location.y, location.z, 1.0f
			}};
	}

	glm::mat3 Transform::normalMatrix()
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		const glm::vec3 invScale = 1.0f / scale;

		return glm::mat3{
			{
				invScale.x* (c1* c3 + s1 * s2 * s3),
				invScale.x* (c2* s3),
				invScale.x* (c1* s2* s3 - c3 * s1),
			},
			{
				invScale.y * (c3 * s1 * s2 - c1 * s3),
				invScale.y * (c2 * c3),
				invScale.y * (c1 * c3 * s2 + s1 * s3),
			},
			{
				invScale.z * (c2 * s1),
				invScale.z * (-s2),
				invScale.z * (c1 * c2),
			}};
	}

	SceneEntity SceneEntity::createEmpty()
	{
		static id_t currentId = 0;
		return SceneEntity{ currentId++ };
	}

	SceneEntity SceneEntity::createPointLight(float intensity, float radius, glm::vec3 color)
	{
		SceneEntity obj = SceneEntity::createEmpty();
		obj.color = color;
		obj.transform.scale.x = radius;
		obj.pointLight = std::make_unique<PointLightComponent>();
		obj.pointLight->lightIntensity = intensity;
		return obj;
	}

	SceneEntity SceneEntity::createModel(std::shared_ptr<VreModel> model)
	{
		SceneEntity obj = SceneEntity::createEmpty();
		obj.model = std::move(model);
		return obj;
	}

} // namespace vre