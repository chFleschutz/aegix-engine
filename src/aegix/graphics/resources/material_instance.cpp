#include "pch.h"

#include "material_instance.h"

namespace Aegix::Graphics
{
	MaterialInstance::MaterialInstance(std::shared_ptr<MaterialTemplate> materialTemplate)
		: m_template(std::move(materialTemplate))
	{
	}

	void MaterialInstance::bindPipeline(VkCommandBuffer cmd) const
	{
		// TODO
	}

	void MaterialInstance::bindDescriptorSet(VkCommandBuffer cmd) const
	{
		// TODO
	}
}
