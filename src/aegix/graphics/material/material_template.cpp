#include "pch.h"
#include "material_template.h"

#include "engine.h"

namespace Aegis::Graphics
{
	MaterialTemplate::MaterialTemplate(Pipeline pipeline)
		: m_pipeline{ std::move(pipeline) }
	{
	}

	auto MaterialTemplate::alignTo(size_t size, size_t alignment) -> size_t
	{
		return (size + alignment - 1) & ~(alignment - 1);
	}

	auto MaterialTemplate::std430Alignment(const MaterialParameter::Value& val) -> size_t
	{
		return std::visit([&](auto&& arg) -> size_t {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, int32_t>)        return 4;
			else if constexpr (std::is_same_v<T, uint32_t>)  return 4;
			else if constexpr (std::is_same_v<T, float>)     return 4;
			else if constexpr (std::is_same_v<T, glm::vec2>) return 8;
			else if constexpr (std::is_same_v<T, glm::vec3>) return 16;
			else if constexpr (std::is_same_v<T, glm::vec4>) return 16;
			else if constexpr (std::is_same_v<T, std::shared_ptr<Texture>>) return sizeof(DescriptorHandle);
			else
			{
				AGX_ASSERT_X(false, "Unknown material parameter type");
				return 16;
			}
		}, val);
	}

	auto MaterialTemplate::std430Size(const MaterialParameter::Value& val) -> size_t
	{
		return std::visit([&](auto&& arg) -> size_t {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, int32_t>)        return 4;
			else if constexpr (std::is_same_v<T, uint32_t>)  return 4;
			else if constexpr (std::is_same_v<T, float>)     return 4;
			else if constexpr (std::is_same_v<T, glm::vec2>) return 8;
			else if constexpr (std::is_same_v<T, glm::vec3>) return 12;
			else if constexpr (std::is_same_v<T, glm::vec4>) return 16;
			else if constexpr (std::is_same_v<T, std::shared_ptr<Texture>>) return sizeof(DescriptorHandle);
			else
			{
				AGX_ASSERT_X(false, "Unknown material parameter type");
				return 16;
			}
		}, val);
	}



	auto MaterialTemplate::hasParameter(const std::string& name) const -> bool
	{
		return m_parameters.contains(name);
	}

	auto MaterialTemplate::queryDefaultParameter(const std::string& name) const -> MaterialParameter::Value
	{
		auto it = m_parameters.find(name);
		if (it != m_parameters.end())
			return it->second.defaultValue;

		AGX_ASSERT_X(false, "Material parameter not found");
		return {};
	}

	void MaterialTemplate::addParameter(const std::string& name, const MaterialParameter::Value& defaultValue)
	{
		AGX_ASSERT_X(!m_parameters.contains(name), "Material parameter already exists");

		MaterialParameter param{
			.offset = alignTo(m_parameterSize, std430Alignment(defaultValue)),
			.size = std430Size(defaultValue),
			.defaultValue = defaultValue,
		};

		if (std::holds_alternative<std::shared_ptr<Texture>>(defaultValue))
		{
			m_textureCount++;
			param.binding = m_textureCount;


		}

		m_parameterSize = param.offset + param.size;
		m_parameters.emplace(name, std::move(param));
	}

	void MaterialTemplate::bind(VkCommandBuffer cmd)
	{
		m_pipeline.bind(cmd);
	}

	void MaterialTemplate::bindBindlessSet(VkCommandBuffer cmd)
	{
		m_pipeline.bindDescriptorSet(cmd, 0, Engine::renderer().bindlessDescriptorSet().descriptorSet());
	}

	void MaterialTemplate::pushConstants(VkCommandBuffer cmd, const void* data, size_t size, uint32_t offset)
	{
		m_pipeline.pushConstants(cmd, VK_SHADER_STAGE_ALL, data, size, offset);
	}

	void MaterialTemplate::draw(VkCommandBuffer cmd, const StaticMesh& mesh)
	{
		if (m_pipeline.hasFlag(Pipeline::Flags::MeshShader))
		{
			mesh.drawMeshlets(cmd);
		}
		else
		{
			mesh.draw(cmd);
		}
	}

	void MaterialTemplate::drawInstanced(VkCommandBuffer cmd, uint32_t instanceCount)
	{
		AGX_ASSERT_X(m_pipeline.hasFlag(Pipeline::Flags::MeshShader), "Instanced draw is currently only supported for mesh shader pipelines");
		vkCmdDrawMeshTasksEXT(cmd, instanceCount, 1, 1);
	}

	void MaterialTemplate::printInfo() const
	{
		ALOG::info("Material Template Info:");
		ALOG::info("  Parameter Size: {} bytes", m_parameterSize);
		ALOG::info("  Parameters:");
		for (const auto& [name, param] : m_parameters)
		{
			ALOG::info("    Name: {}, Binding: {}, Offset: {}, Size: {}",
				name, param.binding, param.offset, param.size);
		}
	}
}
