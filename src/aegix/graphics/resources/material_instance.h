#pragma once

#include "graphics/resources/material_template.h"
#include "graphics/descriptors.h"

namespace Aegix::Graphics
{
	class MaterialInstance
	{
	public:
		MaterialInstance(std::shared_ptr<MaterialTemplate> materialTemplate);
	
		template<typename T>
		[[nodiscard]] auto queryParameter(const std::string& name) const -> T
		{
			return std::get<T>(queryParameter(name));
		}

		[[nodiscard]] auto queryParameter(const std::string& name) const -> MaterialParamValue;
		void setParameter(const std::string& name, const MaterialParamValue& value);

		void updateParameters();

		void bindPipeline(VkCommandBuffer cmd) const;
		void bindDescriptorSet(VkCommandBuffer cmd) const;

	private:
		std::shared_ptr<MaterialTemplate> m_template;
		std::unordered_map<std::string, MaterialParamValue> m_overrides;
		DescriptorSet m_descriptorSet;
		Buffer m_uniformBuffer;
		bool m_dirty = true;
	};
}
