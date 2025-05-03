#include "pch.h"

#include "material_template.h"

namespace Aegix::Graphics
{
	MaterialTemplate::MaterialTemplate(Pipeline pipeline)
		: m_pipeline{ std::move(pipeline) }
	{
	}

	void MaterialTemplate::addShaderStage(VkShaderStageFlagBits stage, const std::filesystem::path& shaderPath)
	{
		// TODO
	}

	void MaterialTemplate::addDesciptor(const std::string& name, uint32_t set, uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags)
	{
		// TODO
	}

	void MaterialTemplate::addPushConstant(VkShaderStageFlags stageFlags, size_t size)
	{
		// TODO
	}

	void MaterialTemplate::addColorAttachment(VkFormat format)
	{
		// TODO
	}

	void MaterialTemplate::setDepthAttachment(VkFormat format)
	{
		// TODO
	}

	void MaterialTemplate::bind()
	{
		// TODO
	}
}
