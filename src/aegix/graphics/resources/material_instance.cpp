#include "pch.h"

#include "material_instance.h"

namespace Aegix::Graphics
{
	MaterialInstance::MaterialInstance(std::shared_ptr<MaterialTemplate> materialTemplate)
		: m_template(std::move(materialTemplate))
	{
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

	void MaterialInstance::bindPipeline(VkCommandBuffer cmd) const
	{
		m_template->bind(cmd);
	}

	void MaterialInstance::bindDescriptorSet(VkCommandBuffer cmd) const
	{
		// TODO
	}
}
