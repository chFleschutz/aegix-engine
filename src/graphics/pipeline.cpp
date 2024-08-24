#include "pipeline.h"

#include "graphics/model.h"
#include "utils/file_utils.h"

#include <cassert>
#include <stdexcept>

namespace Aegix::Graphics
{
	// *************** PipelineLayout::Builder *********************

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


	// *************** PipelineLayout *********************

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

		if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout");
	}

	Graphics::PipelineLayout::~PipelineLayout()
	{
		vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
	}


	// *************** Pipeline::Builder *********************

	Pipeline::Builder::Builder(VulkanDevice& device)
		: m_device{ device }
	{
		Pipeline::defaultPipelineConfigInfo(m_configInfo);
	}

	Pipeline::Builder::~Builder()
	{
		for (auto& shaderStage : m_configInfo.shaderStges)
		{
			vkDestroyShaderModule(m_device.device(), shaderStage.module, nullptr);
		}
	}

	Pipeline::Builder& Pipeline::Builder::setRenderPass(VkRenderPass renderPass)
	{
		m_configInfo.renderPass = renderPass;
		return *this;
	}

	Pipeline::Builder& Pipeline::Builder::setPipelineLayout(VkPipelineLayout pipelineLayout)
	{
		m_configInfo.pipelineLayout = pipelineLayout;
		return *this;
	}

	Pipeline::Builder& Pipeline::Builder::addShaderStage(VkShaderStageFlagBits stage, const std::filesystem::path& shaderPath)
	{
		auto shaderCode = FileUtils::readBinaryFile(shaderPath);

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = shaderCode.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());

		VkShaderModule shaderModule = nullptr;
		if (vkCreateShaderModule(m_device.device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			throw std::runtime_error("failed to create shader module");

		VkPipelineShaderStageCreateInfo shaderStage{};
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = stage;
		shaderStage.module = shaderModule;
		shaderStage.pName = "main";
		shaderStage.flags = 0;
		shaderStage.pNext = nullptr;
		shaderStage.pSpecializationInfo = nullptr;
		m_configInfo.shaderStges.push_back(shaderStage);

		return *this;
	}

	Pipeline::Builder& Pipeline::Builder::enableAlphaBlending()
	{
		Pipeline::enableAlphaBlending(m_configInfo);
		return *this;
	}

	Pipeline::Builder& Pipeline::Builder::setVertexBindingDescriptions(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions)
	{
		m_configInfo.bindingDescriptions = bindingDescriptions;
		return *this;
	}

	Pipeline::Builder& Pipeline::Builder::setVertexAttributeDescriptions(const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
	{
		m_configInfo.attributeDescriptions = attributeDescriptions;
		return *this;
	}

	std::unique_ptr<Pipeline> Pipeline::Builder::build()
	{
		assert(m_configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline: no pipelineLayout provided");
		assert(m_configInfo.renderPass != VK_NULL_HANDLE && "Cannot create pipeline: no renderPass provided");

		return std::make_unique<Pipeline>(m_device, m_configInfo);
	}


	// *************** Pipeline *********************

	Pipeline::Pipeline(VulkanDevice& device, const Pipeline::Config& configInfo)
		: m_device{ device }
	{
		createGraphicsPipeline(configInfo);
	}

	Pipeline::~Pipeline()
	{
		vkDestroyPipeline(m_device.device(), m_graphicsPipeline, nullptr);
	}

	void Pipeline::bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
	}

	void Pipeline::defaultPipelineConfigInfo(Pipeline::Config& configInfo)
	{
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
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

		configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		configInfo.colorBlendAttachment.blendEnable = VK_FALSE;
		configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
		configInfo.colorBlendInfo.attachmentCount = 1;
		configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
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

		configInfo.bindingDescriptions = StaticMesh::Vertex::bindingDescriptions();
		configInfo.attributeDescriptions = StaticMesh::Vertex::attributeDescriptions();
	}

	void Pipeline::enableAlphaBlending(Pipeline::Config& configInfo)
	{
		configInfo.colorBlendAttachment.blendEnable = VK_TRUE;
		configInfo.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	void Pipeline::createGraphicsPipeline(const Pipeline::Config& configInfo)
	{
		assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
		assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline: no renderPass provided in configInfo");

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
		pipelineInfo.renderPass = configInfo.renderPass;
		pipelineInfo.subpass = configInfo.subpass;
		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(m_device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
			throw std::runtime_error("failed to create graphics pipeline");
	}
}