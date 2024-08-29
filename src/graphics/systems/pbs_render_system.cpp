#include "pbs_render_system.h"

namespace Aegix::Graphics
{
	PBSMaterialInstance::PBSMaterialInstance(VulkanDevice& device, DescriptorSetLayout& setLayout, DescriptorPool& pool, PBSMaterial::Data data)
		: m_uniformBuffer{ device, data }
	{
		m_descriptorSet = DescriptorSet::Builder(device, pool, setLayout)
			.addBuffer(1, m_uniformBuffer)
			.build();
	}

	PBSRenderSystem::PBSRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: RenderSystem(device, renderPass, globalSetLayout)
	{
		m_descriptorSetLayout = DescriptorSetLayout::Builder(m_device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		m_pipelineLayout = PipelineLayout::Builder(m_device)
			.addDescriptorSetLayout(globalSetLayout)
			.addDescriptorSetLayout(m_descriptorSetLayout->descriptorSetLayout())
			.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushConstantData))
			.build();

		m_pipeline = Pipeline::Builder(m_device)
			.setRenderPass(renderPass)
			.setPipelineLayout(m_pipelineLayout->pipelineLayout())
			.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "pbs.vert.spv")
			.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "pbs.frag.spv")
			.build();
	}

	void PBSRenderSystem::render(const FrameInfo& frameInfo)
	{

	}
}