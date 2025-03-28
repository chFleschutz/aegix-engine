#include "pch.h"

#include "pipeline.h"

#include "graphics/static_mesh.h"
#include "graphics/vulkan_tools.h"
#include "utils/file.h"

namespace Aegix::Graphics
{
	// PipelineLayout::Builder ---------------------------------------------------

	PipelineLayout::Builder::Builder(VulkanDevice& device)
		: m_device{ device }
	{
	}

	PipelineLayout::Builder& PipelineLayout::Builder::addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
	{
		m_descriptorSetLayouts.push_back(descriptorSetLayout);
		return *this;
	}

	PipelineLayout::Builder& PipelineLayout::Builder::addPushConstantRange(VkPushConstantRange pushConstantRange)
	{
		m_pushConstantRanges.push_back(pushConstantRange);
		return *this;
	}

	PipelineLayout::Builder& PipelineLayout::Builder::addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t size, uint32_t offset)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = stageFlags;
		pushConstantRange.size = size;
		pushConstantRange.offset = offset;
		m_pushConstantRanges.push_back(pushConstantRange);

		return *this;
	}

	std::unique_ptr<PipelineLayout> PipelineLayout::Builder::build()
	{
		return std::make_unique<PipelineLayout>(m_device, m_descriptorSetLayouts, m_pushConstantRanges);
	}


	// PipeLineLayout ------------------------------------------------------------

	Graphics::PipelineLayout::PipelineLayout(VulkanDevice& device, const std::vector<VkDescriptorSetLayout>& setLayouts, 
		const std::vector<VkPushConstantRange>& pushConstants)
		: m_device{ device }
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
		pipelineLayoutInfo.pSetLayouts = setLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
		pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();

		VK_CHECK(vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout))
	}

	Graphics::PipelineLayout::~PipelineLayout()
	{
		vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
	}


	// Pipeline::GraphicsBuilder -------------------------------------------------

	Pipeline::GraphicsBuilder::GraphicsBuilder(VulkanDevice& device, VkPipelineLayout pipelineLayout)
		: m_device{ device }
	{
		Pipeline::defaultGraphicsPipelineConfig(m_config);
		m_config.pipelineLayout = pipelineLayout;
	}

	Pipeline::GraphicsBuilder::~GraphicsBuilder()
	{
		for (auto& shaderStage : m_config.shaderStges)
		{
			vkDestroyShaderModule(m_device.device(), shaderStage.module, nullptr);
		}
	}

	Pipeline::GraphicsBuilder& Pipeline::GraphicsBuilder::addShaderStage(VkShaderStageFlagBits stage, const std::filesystem::path& shaderPath)
	{
		VkShaderModule shaderModule = Tools::createShaderModule(m_device, shaderPath);
		m_config.shaderStges.emplace_back(Tools::createShaderStage(stage, shaderModule));

		return *this;
	}

	Pipeline::GraphicsBuilder& Pipeline::GraphicsBuilder::addColorAttachment(VkFormat colorFormat, bool alphaBlending)
	{
		m_config.colorAttachmentFormats.emplace_back(colorFormat);
		m_config.renderingInfo.colorAttachmentCount = static_cast<uint32_t>(m_config.colorAttachmentFormats.size());
		m_config.renderingInfo.pColorAttachmentFormats = m_config.colorAttachmentFormats.data();

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		if (alphaBlending)
		{
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		}

		m_config.colorBlendAttachments.emplace_back(colorBlendAttachment);
		
		m_config.colorBlendInfo.attachmentCount = m_config.renderingInfo.colorAttachmentCount;
		m_config.colorBlendInfo.pAttachments = m_config.colorBlendAttachments.data();

		return *this;
	}


	Pipeline::GraphicsBuilder& Pipeline::GraphicsBuilder::setDepthAttachment(VkFormat depthFormat)
	{
		m_config.renderingInfo.depthAttachmentFormat = depthFormat;
		return *this;
	}

	Pipeline::GraphicsBuilder& Pipeline::GraphicsBuilder::setStencilFormat(VkFormat stencilFormat)
	{
		m_config.renderingInfo.stencilAttachmentFormat = stencilFormat;
		return *this;
	}

	Pipeline::GraphicsBuilder& Pipeline::GraphicsBuilder::setDepthTest(bool enableDepthTest, bool writeDepth, VkCompareOp compareOp)
	{
		m_config.depthStencilInfo.depthTestEnable = enableDepthTest;
		m_config.depthStencilInfo.depthWriteEnable = writeDepth;
		m_config.depthStencilInfo.depthCompareOp = compareOp;
		return *this;
	}

	Pipeline::GraphicsBuilder& Pipeline::GraphicsBuilder::setCullMode(VkCullModeFlags cullMode)
	{
		m_config.rasterizationInfo.cullMode = cullMode;
		return *this;
	}

	Pipeline::GraphicsBuilder& Pipeline::GraphicsBuilder::setVertexBindingDescriptions(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions)
	{
		m_config.bindingDescriptions = bindingDescriptions;
		return *this;
	}

	Pipeline::GraphicsBuilder& Pipeline::GraphicsBuilder::setVertexAttributeDescriptions(const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		m_config.attributeDescriptions = attributeDescriptions;
		return *this;
	}

	std::unique_ptr<Pipeline> Pipeline::GraphicsBuilder::build()
	{
		return std::make_unique<Pipeline>(m_device, m_config);
	}


	// ComputeBuilder ------------------------------------------------------------

	Pipeline::ComputeBuilder::ComputeBuilder(VulkanDevice& device, VkPipelineLayout pipelineLayout)
		: m_device{ device }
	{
		m_config.pipelineLayout = pipelineLayout;
	}

	Pipeline::ComputeBuilder::~ComputeBuilder()
	{
		if (m_config.shaderStage.module)
		{
			vkDestroyShaderModule(m_device, m_config.shaderStage.module, nullptr);
		}
	}

	Pipeline::ComputeBuilder& Pipeline::ComputeBuilder::setShaderStage(const std::filesystem::path& shaderPath)
	{
		VkShaderModule shaderModule = Tools::createShaderModule(m_device, shaderPath);
		m_config.shaderStage = Tools::createShaderStage(VK_SHADER_STAGE_COMPUTE_BIT, shaderModule);
		return *this;
	}

	std::unique_ptr<Pipeline> Pipeline::ComputeBuilder::build()
	{
		return std::make_unique<Pipeline>(m_device, m_config);
	}


	// Pipeline ------------------------------------------------------------------

	Pipeline::Pipeline(VulkanDevice& device, const Pipeline::GraphicsConfig& config)
		: m_device{ device }, m_bindPoint{ VK_PIPELINE_BIND_POINT_GRAPHICS }
	{
		createGraphicsPipeline(config);
	}

	Pipeline::Pipeline(VulkanDevice& device, const Pipeline::ComputeConfig& config)
		: m_device{ device }, m_bindPoint{ VK_PIPELINE_BIND_POINT_COMPUTE }
	{
		AGX_ASSERT_X(config.pipelineLayout != VK_NULL_HANDLE, "Cannot create pipeline: no pipelineLayout provided");
		AGX_ASSERT_X(config.shaderStage.module != VK_NULL_HANDLE, "Cannot create pipeline: no shader provided");

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = config.shaderStage;
		pipelineInfo.layout = config.pipelineLayout;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VK_CHECK(vkCreateComputePipelines(m_device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline));
	}

	Pipeline::~Pipeline()
	{
		vkDestroyPipeline(m_device.device(), m_pipeline, nullptr);
	}

	void Pipeline::bind(VkCommandBuffer commandBuffer) const
	{
		vkCmdBindPipeline(commandBuffer, m_bindPoint, m_pipeline);
	}

	void Pipeline::defaultGraphicsPipelineConfig(Pipeline::GraphicsConfig& configInfo)
	{
		configInfo.renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

		configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		configInfo.viewportInfo.viewportCount = 1;
		configInfo.viewportInfo.pViewports = nullptr;
		configInfo.viewportInfo.scissorCount = 1;
		configInfo.viewportInfo.pScissors = nullptr;

		configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
		configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		configInfo.rasterizationInfo.lineWidth = 1.0f;
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
		configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;
		configInfo.rasterizationInfo.depthBiasClamp = 0.0f;
		configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;

		configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
		configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configInfo.multisampleInfo.minSampleShading = 1.0f;
		configInfo.multisampleInfo.pSampleMask = nullptr;
		configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

		configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
		configInfo.colorBlendInfo.attachmentCount = static_cast<uint32_t>(configInfo.colorBlendAttachments.size());
		configInfo.colorBlendInfo.pAttachments = configInfo.colorBlendAttachments.data();
		configInfo.colorBlendInfo.blendConstants[0] = 0.0f;
		configInfo.colorBlendInfo.blendConstants[1] = 0.0f;
		configInfo.colorBlendInfo.blendConstants[2] = 0.0f;
		configInfo.colorBlendInfo.blendConstants[3] = 0.0f;

		configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.minDepthBounds = 0.0f;
		configInfo.depthStencilInfo.maxDepthBounds = 1.0f;
		configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.front = {};
		configInfo.depthStencilInfo.back = {};

		configInfo.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
		configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
		configInfo.dynamicStateInfo.flags = 0;

		configInfo.bindingDescriptions = StaticMesh::defaultBindingDescriptions();
		configInfo.attributeDescriptions = StaticMesh::defaultAttributeDescriptions();
	}

	void Pipeline::createGraphicsPipeline(const Pipeline::GraphicsConfig& configInfo)
	{
		AGX_ASSERT_X(configInfo.pipelineLayout != VK_NULL_HANDLE, "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");

		auto& bindingDescriptions = configInfo.bindingDescriptions;
		auto& attributeDescriptions = configInfo.attributeDescriptions;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = &configInfo.renderingInfo;
		pipelineInfo.stageCount = static_cast<uint32_t>(configInfo.shaderStges.size());
		pipelineInfo.pStages = configInfo.shaderStges.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &configInfo.viewportInfo;
		pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
		pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;
		pipelineInfo.layout = configInfo.pipelineLayout;
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = configInfo.subpass;
		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VK_CHECK(vkCreateGraphicsPipelines(m_device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline));
	}
}