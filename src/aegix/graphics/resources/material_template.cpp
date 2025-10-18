#include "pch.h"
#include "material_template.h"

namespace Aegix::Graphics
{
	MaterialTemplate::MaterialTemplate(Pipeline pipeline, DescriptorSetLayout globalSetLayout, DescriptorSetLayout materialSetLayout)
		: m_pipeline{ std::move(pipeline) },
		m_globalSetLayout{ std::move(globalSetLayout) },
		m_materialSetLayout{ std::move(materialSetLayout) }
	{
	}

	auto MaterialTemplate::alignTo(size_t size, size_t alignment) -> size_t
	{
		return (size + alignment - 1) & ~(alignment - 1);
	}

	auto MaterialTemplate::std140Alignment(MaterialParamType type) -> size_t
	{
		switch (type)
		{
		case MaterialParamType::Int: return 4;
		case MaterialParamType::Float: return 4;
		case MaterialParamType::Vec2: return 8;
		case MaterialParamType::Vec3: return 16;
		case MaterialParamType::Vec4: return 16;
		default: return 16;
		}
	}

	auto MaterialTemplate::std140Size(MaterialParamType type) -> size_t
	{
		switch (type)
		{
		case MaterialParamType::Int: return 4;
		case MaterialParamType::Float: return 4;
		case MaterialParamType::Vec2: return 8;
		case MaterialParamType::Vec3: return 12;
		case MaterialParamType::Vec4: return 16;
		default: return 16;
		}
	}



	auto MaterialTemplate::hasParameter(const std::string& name) const -> bool
	{
		return m_parameters.contains(name);
	}

	auto MaterialTemplate::queryDefaultParameter(const std::string& name) const -> MaterialParamValue
	{
		auto it = m_parameters.find(name);
		if (it != m_parameters.end())
			return it->second.defaultValue;

		AGX_ASSERT_X(false, "Material parameter not found");
		return {};
	}

	void MaterialTemplate::addParameter(const std::string& name, MaterialParamType type, const MaterialParamValue& defaultValue)
	{
		AGX_ASSERT_X(!m_parameters.contains(name), "Material parameter already exists");

		MaterialParameter param{
			.type = type,
			.binding = 0,
			.offset = 0,
			.size = 0,
			.defaultValue = defaultValue,
		};

		if (type == MaterialParamType::Texture2D)
		{
			m_textureCount++;
			param.binding = m_textureCount;
		}
		else
		{
			param.size = std140Size(type);
			param.offset = alignTo(m_parameterSize, std140Alignment(type));
			m_parameterSize = param.offset + param.size;
		}

		m_parameters.emplace(name, std::move(param));
	}

	void MaterialTemplate::bind(VkCommandBuffer cmd)
	{
		m_pipeline.bind(cmd);
	}

	void MaterialTemplate::bindGlobalSet(VkCommandBuffer cmd, VkDescriptorSet descriptorSet)
	{
		m_pipeline.bindDescriptorSet(cmd, 0, descriptorSet);
	}

	void MaterialTemplate::bindMaterialSet(VkCommandBuffer cmd, VkDescriptorSet descriptorSet)
	{
		m_pipeline.bindDescriptorSet(cmd, 1, descriptorSet);
	}

	void MaterialTemplate::pushConstants(VkCommandBuffer cmd, const void* data, size_t size, uint32_t offset)
	{
		m_pipeline.pushConstants(cmd, VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT, data, size, offset);
	}

	void MaterialTemplate::draw(VkCommandBuffer cmd, const StaticMesh& mesh)
	{
		if (m_pipeline.hasFlag(Pipeline::Flags::MeshShader))
		{
			m_pipeline.bindDescriptorSet(cmd, 2, mesh.meshletDescriptorSet());
			m_pipeline.bindDescriptorSet(cmd, 3, mesh.attributeDescriptorSet());
			mesh.drawMeshlets(cmd);
		}
		else
		{
			mesh.draw(cmd);
		}
	}
}
