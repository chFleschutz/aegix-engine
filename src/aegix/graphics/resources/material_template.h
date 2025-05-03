#pragma once

#include "graphics/pipeline.h"
#include "graphics/resources/texture.h"
#include "math/math.h"

#include <variant>

namespace Aegix::Graphics
{
	using MaterialParamValue = std::variant<
		int, 
		float, 
		glm::vec2, 
		glm::vec3, 
		glm::vec4, 
		std::shared_ptr<Texture>
	>;

	enum class MaterialParamType
	{
		Int,
		Float,
		Vec2,
		Vec3,
		Vec4,
		Texture
	};

	struct MaterialParameter
	{
		MaterialParamType type;
		size_t offset = 0;
		size_t size = 0;
		MaterialParamValue defaultValue;
	};

	class MaterialTemplate
	{
	public:
		MaterialTemplate(Pipeline pipeline);

		[[nodiscard]] static auto alignTo(size_t size, size_t alignment) -> size_t;
		[[nodiscard]] static auto std140Alignment(MaterialParamType type) -> size_t;
		[[nodiscard]] static auto std140Size(MaterialParamType type) -> size_t;

		[[nodiscard]] auto hasParameter(const std::string& name) const -> bool;
		[[nodiscard]] auto queryDefaultParameter(const std::string& name) const -> MaterialParamValue;
		void addParameter(const std::string& name, MaterialParamType type, const MaterialParamValue& defaultValue);

		void bind(VkCommandBuffer cmd);

	private:
		Pipeline m_pipeline;
		size_t m_parameterSize = 0;
		std::unordered_map<std::string, MaterialParameter> m_parameters;
	};
}
