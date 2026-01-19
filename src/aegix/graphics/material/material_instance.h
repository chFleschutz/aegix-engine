#pragma once

#include "core/asset.h"
#include "graphics/descriptors.h"
#include "graphics/material/material_template.h"
#include "graphics/bindless/bindless_buffer.h"

namespace Aegis::Graphics
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

		template<typename T>
		[[nodiscard]] auto queryParameter(const std::string& name) const -> T
		{
			return std::get<T>(queryParameter(name));
		}

		[[nodiscard]] auto materialTemplate() const -> std::shared_ptr<MaterialTemplate> { return m_template; }
		[[nodiscard]] auto queryParameter(const std::string& name) const -> MaterialParameter::Value;
		[[nodiscard]] auto buffer() const -> const BindlessFrameBuffer& { return m_uniformBuffer; }

		void setParameter(const std::string& name, const MaterialParameter::Value& value);
		void updateParameters(int index);

	private:
		std::shared_ptr<MaterialTemplate> m_template;
		std::unordered_map<std::string, MaterialParameter::Value> m_overrides;
		BindlessFrameBuffer m_uniformBuffer;
		std::array<bool, MAX_FRAMES_IN_FLIGHT> m_dirtyFlags;
	};
}
