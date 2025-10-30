#include "pch.h"
#include "material_instance.h"

#include "graphics/vulkan/vulkan_context.h"

namespace Aegix::Graphics
{
	MaterialInstance::MaterialInstance(std::shared_ptr<MaterialTemplate> materialTemplate) : 
		m_template(std::move(materialTemplate)), 
		m_uniformBuffer{ Buffer::createUniformBuffer(m_template->parameterSize(), MAX_FRAMES_IN_FLIGHT) } 
	{
		AGX_ASSERT_X(m_template, "Material template cannot be null");

		m_descriptorSets.reserve(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_descriptorSets.emplace_back(m_template->materialSetLayout());
			DescriptorWriter{ m_template->materialSetLayout() }
				.writeBuffer(0, m_uniformBuffer, i)
				.update(m_descriptorSets[i]);
		}

		m_dirtyFlags.fill(true);
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
		m_dirtyFlags.fill(true);
	}

	void MaterialInstance::updateParameters(int index)
	{
		if (!m_dirtyFlags[index])
			return;

		// Update uniform buffer
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
		
		// Update descriptor set (Textures may have changed)
		DescriptorWriter writer{ m_template->materialSetLayout() };
		writer.writeBuffer(0, m_uniformBuffer);
		for (const auto& [name, info] : m_template->parameters())
		{
			if (info.type != MaterialParamType::Texture2D)
				continue;

			auto it = m_overrides.find(name);
			if (it != m_overrides.end())
			{
				auto& texture = std::get<std::shared_ptr<Texture>>(it->second);
				writer.writeImage(info.binding, *texture);
			}
			else
			{
				auto& texture = std::get<std::shared_ptr<Texture>>(info.defaultValue);
				writer.writeImage(info.binding, *texture);
			}
		}
		writer.update(m_descriptorSets[index]);

		m_dirtyFlags[index] = false;
	}

	void MaterialInstance::bind(VkCommandBuffer cmd, int index) const
	{
		m_template->bindMaterialSet(cmd, m_descriptorSets[index]);
	}
}
