#pragma once

#include "core/asset.h"
#include "graphics/descriptors.h"
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
		Texture2D
	};

	struct MaterialParameter
	{
		MaterialParamType type;
		uint32_t binding = 0;
		size_t offset = 0;
		size_t size = 0;
		MaterialParamValue defaultValue;
	};

	class MaterialTemplate : public Core::Asset
	{
	public:
		MaterialTemplate(Pipeline pipeline, DescriptorSetLayout globalSetLayout, DescriptorSetLayout materialSetLayout);

		[[nodiscard]] static auto alignTo(size_t size, size_t alignment) -> size_t;
		[[nodiscard]] static auto std140Alignment(MaterialParamType type) -> size_t;
		[[nodiscard]] static auto std140Size(MaterialParamType type) -> size_t;

		[[nodiscard]] auto pipeline() const -> const Pipeline& { return m_pipeline; }
		[[nodiscard]] auto globalSetLayout() -> DescriptorSetLayout& { return m_globalSetLayout; }
		[[nodiscard]] auto materialSetLayout() -> DescriptorSetLayout& { return m_materialSetLayout; }
		[[nodiscard]] auto hasParameter(const std::string& name) const -> bool;
		[[nodiscard]] auto parameterSize() const -> size_t { return m_parameterSize; }
		[[nodiscard]] auto parameters() const -> const std::unordered_map<std::string, MaterialParameter>& { return m_parameters; }
		[[nodiscard]] auto queryDefaultParameter(const std::string& name) const -> MaterialParamValue;

		void addParameter(const std::string& name, MaterialParamType type, const MaterialParamValue& defaultValue);

		void bind(VkCommandBuffer cmd);
		void bindGlobalSet(VkCommandBuffer cmd, VkDescriptorSet descriptorSet);
		void bindMaterialSet(VkCommandBuffer cmd, VkDescriptorSet descriptorSet);
		void pushConstants(VkCommandBuffer cmd, const void* data, size_t size, uint32_t offset = 0);

	private:
		Pipeline m_pipeline;
		DescriptorSetLayout m_globalSetLayout;		// Set 0 // TODO: check if this is actually needed here
		DescriptorSetLayout m_materialSetLayout;	// Set 1
		std::unordered_map<std::string, MaterialParameter> m_parameters;
		size_t m_parameterSize = 0;
		uint32_t m_textureCount = 0;
	};
}
