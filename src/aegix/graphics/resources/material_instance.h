#pragma once

#include "graphics/resources/material_template.h"

namespace Aegix::Graphics
{
	class MaterialInstance
	{
	public:
		MaterialInstance(std::shared_ptr<MaterialTemplate> materialTemplate);
	
		template <typename T>
		void set(const std::string& name, const T& value)
		{
			// TODO
		}

		void bindPipeline(VkCommandBuffer cmd) const;
		void bindDescriptorSet(VkCommandBuffer cmd) const;

	private:
		std::shared_ptr<MaterialTemplate> m_template;
	};
}
