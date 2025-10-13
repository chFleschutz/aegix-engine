#pragma once

#include "core/asset.h"
#include "graphics/descriptors.h"
#include "graphics/resources/material_template.h"

namespace Aegix::Graphics
{
	class MaterialInstance : public Core::Asset
	{
	public:
		static auto create(std::shared_ptr<MaterialTemplate> materialTemplate) -> std::shared_ptr<MaterialInstance>
		{
			return std::make_shared<MaterialInstance>(std::move(materialTemplate));
		}

		MaterialInstance(std::shared_ptr<MaterialTemplate> materialTemplate);
		MaterialInstance(const MaterialInstance&) = delete;
		MaterialInstance(MaterialInstance&&) = delete;
		~MaterialInstance() = default;

		auto operator=(const MaterialInstance&) -> MaterialInstance& = delete;
		auto operator=(MaterialInstance&&) noexcept -> MaterialInstance& = delete;
	
		[[nodiscard]] auto materialTemplate() const -> std::shared_ptr<MaterialTemplate> { return m_template; }
		[[nodiscard]] auto queryParameter(const std::string& name) const -> MaterialParamValue;

		template<typename T>
		[[nodiscard]] auto queryParameter(const std::string& name) const -> T
		{
			return std::get<T>(queryParameter(name));
		}

		void setParameter(const std::string& name, const MaterialParamValue& value);

		void updateParameters();
		void bind(VkCommandBuffer cmd) const;

	private:
		std::shared_ptr<MaterialTemplate> m_template;
		std::unordered_map<std::string, MaterialParamValue> m_overrides;
		VkDescriptorSet m_descriptorSet{ VK_NULL_HANDLE };
		Buffer m_uniformBuffer;
		bool m_dirty = true;
	};
}
