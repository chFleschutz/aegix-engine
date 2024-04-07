#include "render_system.h"

namespace VEGraphics
{
	void RenderSystem::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(m_DescriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = m_DescriptorSetLayouts.data();
		// TODO: Setup push constants similar to descriptor set layouts
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout");
	}

	void RenderSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(mPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = mPipelineLayout;

		// TODO: dont hardcode shader paths here
		std::string vertShaderPath = SHADER_DIR "simple_shader.vert.spv";
		std::string fragShaderPath = SHADER_DIR "simple_shader.frag.spv";

		mPipeline = std::make_unique<Pipeline>(m_device, vertShaderPath, fragShaderPath, pipelineConfig);
	}
}
