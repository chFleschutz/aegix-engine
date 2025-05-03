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
		case MaterialParamType::Vec3: return 16;
		case MaterialParamType::Vec4: return 16;
		default: return 16;
		}
	}

	void MaterialTemplate::bind(VkCommandBuffer cmd)
	{
		m_pipeline.bind(cmd);
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
		m_parameterSize = alignTo(m_parameterSize, std140Alignment(type));

		m_parameters[name] = MaterialParameter{
			.type = type,
			.offset = m_parameterSize,
			.size = std140Size(type),
			.defaultValue = defaultValue,
		};

		m_parameterSize += m_parameters[name].size;
	}
}
