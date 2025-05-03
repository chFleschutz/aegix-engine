#include "pch.h"

#include "material_instance.h"

namespace Aegix::Graphics
{
	MaterialInstance::MaterialInstance(std::shared_ptr<MaterialTemplate> materialTemplate)
		: m_template(std::move(materialTemplate)), m_uniformBuffer{ Buffer::createUniformBuffer(m_template->parameterSize(), 1) }
	{
		updateParameters();
	}

	auto MaterialInstance::queryParameter(const std::string& name) const -> MaterialParamValue
	{
		auto it = m_overrides.find(name);
		if (it != m_overrides.end())
			return it->second;

		return m_template->queryDefaultParameter(name);
	}

	void MaterialInstance::setParameter(const std::string& name, const MaterialParamValue& value)
	{
		AGX_ASSERT_X(m_template->hasParameter(name), "Material parameter not found");
		m_overrides[name] = value;
	}

	void MaterialInstance::updateParameters()
	{
		if (!m_dirty)
			return;

		m_uniformBuffer.map();

		for (const auto& [name, info] : m_template->parameters())
		{
			auto it = m_overrides.find(name);
			if (it != m_overrides.end())
			{
				const auto& value = it->second;
				m_uniformBuffer.write(&value, info.size, info.offset);
			}
			else
			{
				const auto& defaultValue = info.defaultValue;
				m_uniformBuffer.write(&defaultValue, info.size, info.offset);
			}
		}

		m_uniformBuffer.unmap();
		m_dirty = false;
	}

	void MaterialInstance::bindPipeline(VkCommandBuffer cmd) const
	{
		m_template->bind(cmd);
	}

	void MaterialInstance::bindDescriptorSet(VkCommandBuffer cmd) const
	{
		// TODO
	}
}
