#include "pch.h"
#include "material_instance.h"

#include "graphics/vulkan/vulkan_context.h"

namespace Aegix::Graphics
{
	MaterialInstance::MaterialInstance(std::shared_ptr<MaterialTemplate> materialTemplate) : 
		m_template(std::move(materialTemplate)), 
		m_uniformBuffer{ Buffer::uniformBuffer(m_template->parameterSize(), MAX_FRAMES_IN_FLIGHT) } 
	{
		AGX_ASSERT_X(m_template, "Material template cannot be null");

		m_dirtyFlags.fill(true);
	}

	auto MaterialInstance::queryParameter(const std::string& name) const -> MaterialParameter::Value
	{
		auto it = m_overrides.find(name);
		if (it != m_overrides.end())
			return it->second;

		return m_template->queryDefaultParameter(name);
	}

	void MaterialInstance::setParameter(const std::string& name, const MaterialParameter::Value& value)
	{
		AGX_ASSERT_X(m_template->hasParameter(name), "Material parameter not found");
		m_overrides[name] = value;
		m_dirtyFlags.fill(true);
	}

	void MaterialInstance::updateParameters(int index)
	{
		if (!m_dirtyFlags[index])
			return;

		// Update uniform buffer
		for (const auto& [name, info] : m_template->parameters())
		{
			const MaterialParameter::Value* valuePtr = &info.defaultValue;
			auto it = m_overrides.find(name);
			if (it != m_overrides.end())
			{
				valuePtr = &it->second;
			}

			if (auto texture = std::get_if<std::shared_ptr<Texture>>(valuePtr))
			{
				auto handle = (*texture)->sampledDescriptorHandle();
				AGX_ASSERT_X(handle.isValid(), "Invalid texture descriptor handle in MaterialInstance!");
				m_uniformBuffer.write(&handle, info.size, info.offset, index);
			}
			else
			{
				m_uniformBuffer.write(valuePtr, info.size, info.offset, index);
			}
		}
		
		m_dirtyFlags[index] = false;
	}
}
