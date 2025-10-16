#include "pch.h"

#include "pipeline.h"

#include "graphics/resources/static_mesh.h"
#include "graphics/vulkan/vulkan_context.h"
#include "graphics/vulkan/vulkan_tools.h"

namespace Aegix::Graphics
{
	// Pipeline::GraphicsBuilder -------------------------------------------------

	Pipeline::GraphicsBuilder::GraphicsBuilder()
	{
		Pipeline::defaultGraphicsPipelineConfig(m_graphicsConfig);
	}

	Pipeline::GraphicsBuilder::~GraphicsBuilder()
	{
		for (auto& shaderStage : m_graphicsConfig.shaderStges)
		{
			vkDestroyShaderModule(VulkanContext::device(), shaderStage.module, nullptr);
		}
	}

	auto Pipeline::GraphicsBuilder::addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) -> GraphicsBuilder&
	{
		m_layoutConfig.descriptorSetLayouts.push_back(descriptorSetLayout);
		return *this;
	}

	auto Pipeline::GraphicsBuilder::addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t size, uint32_t offset) -> GraphicsBuilder&
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = stageFlags;
		pushConstantRange.size = size;
		pushConstantRange.offset = offset;
		m_layoutConfig.pushConstantRanges.emplace_back(pushConstantRange);
		return *this;
	}

	auto Pipeline::GraphicsBuilder::addShaderStage(VkShaderStageFlagBits stage, const std::filesystem::path& shaderPath) -> Pipeline::GraphicsBuilder&
	{
		VkShaderModule shaderModule = Tools::createShaderModule(VulkanContext::device(), shaderPath);
		m_graphicsConfig.shaderStges.emplace_back(Tools::createShaderStage(stage, shaderModule));

		return *this;
	}

	auto Pipeline::GraphicsBuilder::addColorAttachment(VkFormat colorFormat, bool alphaBlending) -> Pipeline::GraphicsBuilder&
	{
		m_graphicsConfig.colorAttachmentFormats.emplace_back(colorFormat);
		m_graphicsConfig.renderingInfo.colorAttachmentCount = static_cast<uint32_t>(m_graphicsConfig.colorAttachmentFormats.size());
		m_graphicsConfig.renderingInfo.pColorAttachmentFormats = m_graphicsConfig.colorAttachmentFormats.data();

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

		m_graphicsConfig.colorBlendAttachments.emplace_back(colorBlendAttachment);

		m_graphicsConfig.colorBlendInfo.attachmentCount = m_graphicsConfig.renderingInfo.colorAttachmentCount;
		m_graphicsConfig.colorBlendInfo.pAttachments = m_graphicsConfig.colorBlendAttachments.data();

		return *this;
	}


	auto Pipeline::GraphicsBuilder::setDepthAttachment(VkFormat depthFormat) -> Pipeline::GraphicsBuilder&
	{
		m_graphicsConfig.renderingInfo.depthAttachmentFormat = depthFormat;
		return *this;
	}

	auto Pipeline::GraphicsBuilder::setStencilFormat(VkFormat stencilFormat) -> Pipeline::GraphicsBuilder&
	{
		m_graphicsConfig.renderingInfo.stencilAttachmentFormat = stencilFormat;
		return *this;
	}

	auto Pipeline::GraphicsBuilder::setDepthTest(bool enableDepthTest, bool writeDepth, VkCompareOp compareOp) -> Pipeline::GraphicsBuilder&
	{
		m_graphicsConfig.depthStencilInfo.depthTestEnable = enableDepthTest;
		m_graphicsConfig.depthStencilInfo.depthWriteEnable = writeDepth;
		m_graphicsConfig.depthStencilInfo.depthCompareOp = compareOp;
		return *this;
	}

	auto Pipeline::GraphicsBuilder::setCullMode(VkCullModeFlags cullMode) -> Pipeline::GraphicsBuilder&
	{
		m_graphicsConfig.rasterizationInfo.cullMode = cullMode;
		return *this;
	}

	auto Pipeline::GraphicsBuilder::setVertexBindingDescriptions(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions) -> Pipeline::GraphicsBuilder&
	{
		m_graphicsConfig.bindingDescriptions = bindingDescriptions;
		return *this;
	}

	auto Pipeline::GraphicsBuilder::setVertexAttributeDescriptions(const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
		-> Pipeline::GraphicsBuilder&
	{
		m_graphicsConfig.attributeDescriptions = attributeDescriptions;
		return *this;
	}

	auto Pipeline::GraphicsBuilder::disableVertexInput() -> GraphicsBuilder&
	{
		m_graphicsConfig.useVertexInput = false;
		return *this;
	}

	auto Pipeline::GraphicsBuilder::buildUnique() -> std::unique_ptr<Pipeline>
	{
		return std::make_unique<Pipeline>(m_layoutConfig, m_graphicsConfig);
	}

	auto Pipeline::GraphicsBuilder::build() -> Pipeline
	{
		return Pipeline{ m_layoutConfig, m_graphicsConfig };
	}

	// ComputeBuilder ------------------------------------------------------------

	Pipeline::ComputeBuilder::~ComputeBuilder()
	{
		if (m_computeConfig.shaderStage.module)
		{
			vkDestroyShaderModule(VulkanContext::device(), m_computeConfig.shaderStage.module, nullptr);
		}
	}

	auto Pipeline::ComputeBuilder::addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) -> ComputeBuilder&
	{
		m_layoutConfig.descriptorSetLayouts.emplace_back(descriptorSetLayout);
		return *this;
	}

	auto Pipeline::ComputeBuilder::addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t size) -> ComputeBuilder&
	{
		AGX_ASSERT_X(size <= VulkanContext::device().properties().limits.maxPushConstantsSize, "Push constant size exceeds device limits");

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = stageFlags;
		pushConstantRange.size = size;
		pushConstantRange.offset = 0;
		m_layoutConfig.pushConstantRanges.emplace_back(pushConstantRange);
		return *this;
	}

	auto Pipeline::ComputeBuilder::setShaderStage(const std::filesystem::path& shaderPath) -> Pipeline::ComputeBuilder&
	{
		VkShaderModule shaderModule = Tools::createShaderModule(VulkanContext::device(), shaderPath);
		m_computeConfig.shaderStage = Tools::createShaderStage(VK_SHADER_STAGE_COMPUTE_BIT, shaderModule);
		return *this;
	}

	auto Pipeline::ComputeBuilder::buildUnique() -> std::unique_ptr<Pipeline>
	{
		return std::make_unique<Pipeline>(m_layoutConfig, m_computeConfig);
	}

	auto Pipeline::ComputeBuilder::build() -> Pipeline
	{
		return Pipeline{ m_layoutConfig, m_computeConfig };
	}

	// Pipeline ------------------------------------------------------------------

	Pipeline::Pipeline(const LayoutConfig& layoutConfig, const GraphicsConfig& graphicsConfig)
		: m_bindPoint{ VK_PIPELINE_BIND_POINT_GRAPHICS }
	{
		createPipelineLayout(layoutConfig);
		createGraphicsPipeline(graphicsConfig);
	}

	Pipeline::Pipeline(const LayoutConfig& layoutConfig, const ComputeConfig& computeConfig)
		: m_bindPoint{ VK_PIPELINE_BIND_POINT_COMPUTE }
	{
		createPipelineLayout(layoutConfig);
		createComputePipeline(computeConfig);
	}

	Pipeline::Pipeline(Pipeline&& other) noexcept
		: m_layout{ other.m_layout }, m_pipeline{other.m_pipeline}, m_bindPoint{ other.m_bindPoint }
	{
		other.m_layout = VK_NULL_HANDLE;
		other.m_pipeline = VK_NULL_HANDLE;
	}

	Pipeline::~Pipeline()
	{
		destroy();
	}

	auto Pipeline::operator=(Pipeline&& other) noexcept -> Pipeline&
	{
		if (this != &other)
		{
			destroy();
			m_layout = other.m_layout;
			m_pipeline = other.m_pipeline;
			m_bindPoint = other.m_bindPoint;
			other.m_layout = VK_NULL_HANDLE;
			other.m_pipeline = VK_NULL_HANDLE;
		}
		return *this;
	}

	void Pipeline::bind(VkCommandBuffer commandBuffer) const
	{
		vkCmdBindPipeline(commandBuffer, m_bindPoint, m_pipeline);
	}

	void Pipeline::bindDescriptorSet(VkCommandBuffer cmd, uint32_t setIndex, VkDescriptorSet descriptorSet) const
	{
		vkCmdBindDescriptorSets(cmd, m_bindPoint, m_layout, setIndex, 1, &descriptorSet, 0, nullptr);
	}

	void Pipeline::pushConstants(VkCommandBuffer cmd, VkShaderStageFlags stageFlags, const void* data, uint32_t size, uint32_t offset) const
	{
		vkCmdPushConstants(cmd, m_layout, stageFlags, offset, size, data);
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

	void Pipeline::createPipelineLayout(const LayoutConfig& config)
	{
		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = static_cast<uint32_t>(config.descriptorSetLayouts.size());
		layoutInfo.pSetLayouts = config.descriptorSetLayouts.data();
		layoutInfo.pushConstantRangeCount = static_cast<uint32_t>(config.pushConstantRanges.size());
		layoutInfo.pPushConstantRanges = config.pushConstantRanges.data();

		VK_CHECK(vkCreatePipelineLayout(VulkanContext::device(), &layoutInfo, nullptr, &m_layout));
	}

	void Pipeline::createGraphicsPipeline(const Pipeline::GraphicsConfig& config)
	{
		auto& bindingDescriptions = config.bindingDescriptions;
		auto& attributeDescriptions = config.attributeDescriptions;
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = &config.renderingInfo;
		pipelineInfo.stageCount = static_cast<uint32_t>(config.shaderStges.size());
		pipelineInfo.pStages = config.shaderStges.data();
		pipelineInfo.pVertexInputState = config.useVertexInput ? &vertexInputInfo : nullptr;
		pipelineInfo.pInputAssemblyState = config.useVertexInput ? &config.inputAssemblyInfo : nullptr;
		pipelineInfo.pViewportState = &config.viewportInfo;
		pipelineInfo.pRasterizationState = &config.rasterizationInfo;
		pipelineInfo.pMultisampleState = &config.multisampleInfo;
		pipelineInfo.pColorBlendState = &config.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &config.depthStencilInfo;
		pipelineInfo.pDynamicState = &config.dynamicStateInfo;
		pipelineInfo.layout = m_layout;
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = config.subpass;
		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		VK_CHECK(vkCreateGraphicsPipelines(VulkanContext::device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline));
	}

	void Pipeline::createComputePipeline(const ComputeConfig& config)
	{
		AGX_ASSERT_X(config.shaderStage.module != VK_NULL_HANDLE, "Cannot create pipeline: no shader provided");

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.stage = config.shaderStage;
		pipelineInfo.layout = m_layout;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VK_CHECK(vkCreateComputePipelines(VulkanContext::device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline));
	}

	void Pipeline::destroy()
	{
		VulkanContext::destroy(m_pipeline);
		m_pipeline = VK_NULL_HANDLE;

		VulkanContext::destroy(m_layout);
		m_layout = VK_NULL_HANDLE;
	}
}