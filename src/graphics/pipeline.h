#pragma once

#include "graphics/device.h"

#include <filesystem>
#include <vector>

namespace Aegix::Graphics
{
	struct PipelineConfigInfo
	{
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		std::vector<VkPipelineShaderStageCreateInfo> shaderStges{};

		VkPipelineViewportStateCreateInfo viewportInfo{};
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
		VkPipelineMultisampleStateCreateInfo multisampleInfo{};
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
		std::vector<VkDynamicState> dynamicStateEnables{};
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class Pipeline
	{
	public:
		class Builder
		{
		public:
			Builder(VulkanDevice& device);
			~Builder();

			Builder& setRenderPass(VkRenderPass renderPass);
			Builder& setPipelineLayout(VkPipelineLayout pipelineLayout);
			Builder& addShaderStage(VkShaderStageFlagBits stage, const std::filesystem::path& shaderPath);
			Builder& enableAlphaBlending();

			std::unique_ptr<Pipeline> build();

		private:
			VulkanDevice& m_device;
			PipelineConfigInfo m_configInfo;
		};

		Pipeline(VulkanDevice& device, const PipelineConfigInfo& configInfo);
		Pipeline(const Pipeline&) = delete;
		Pipeline operator=(const Pipeline&) = delete;
		~Pipeline();

		void bind(VkCommandBuffer commandBuffer);

		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
		static void enableAlphaBlending(PipelineConfigInfo& configInfo);

	private:
		void createGraphicsPipeline(const PipelineConfigInfo& configInfo);

		VulkanDevice& m_device;
		VkPipeline m_graphicsPipeline;
	};
}
