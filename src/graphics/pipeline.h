#pragma once

#include "graphics/device.h"

#include <filesystem>
#include <vector>

namespace Aegix::Graphics
{
	class PipelineLayout
	{
	public:
		class Builder
		{
		public:
			Builder(VulkanDevice& device);
			~Builder() = default;

			Builder& addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
			Builder& addPushConstantRange(VkPushConstantRange pushConstantRange);
			Builder& addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t size, uint32_t offset = 0);

			std::unique_ptr<PipelineLayout> build();

		private:
			VulkanDevice& m_device;
			std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
			std::vector<VkPushConstantRange> m_pushConstantRanges;
		};

		PipelineLayout(VulkanDevice& device, const std::vector<VkDescriptorSetLayout>& setLayouts, 
			const std::vector<VkPushConstantRange>& pushConstants);
		PipelineLayout(const PipelineLayout&) = delete;
		~PipelineLayout();

		PipelineLayout operator=(const PipelineLayout&) = delete;

		operator VkPipelineLayout() const { return m_pipelineLayout; }
		VkPipelineLayout pipelineLayout() const { return m_pipelineLayout; }

	private:
		VulkanDevice& m_device;
		VkPipelineLayout m_pipelineLayout;
	};



	class Pipeline
	{
	public:
		struct Config
		{
			Config() = default;
			Config(const Config&) = delete;
			Config& operator=(const Config&) = delete;

			std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
			std::vector<VkPipelineShaderStageCreateInfo> shaderStges{};
			VkPipelineRenderingCreateInfo renderingInfo{};
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
			uint32_t subpass = 0;
		};

		class Builder
		{
		public:
			Builder(VulkanDevice& device, VkPipelineLayout pipelineLayout);
			~Builder();

			Builder& addShaderStage(VkShaderStageFlagBits stage, const std::filesystem::path& shaderPath);
			Builder& setColorAttachmentFormats(std::vector<VkFormat> colorFormats);
			Builder& setDepthAttachmentFormat(VkFormat depthFormat);
			Builder& setStencilFormat(VkFormat stencilFormat);
			Builder& enableAlphaBlending();
			Builder& setVertexBindingDescriptions(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions);
			Builder& setVertexAttributeDescriptions(const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);

			std::unique_ptr<Pipeline> build();

		private:
			VulkanDevice& m_device;
			Pipeline::Config m_configInfo;
		};

		Pipeline(VulkanDevice& device, const Pipeline::Config& configInfo);
		Pipeline(const Pipeline&) = delete;
		Pipeline operator=(const Pipeline&) = delete;
		~Pipeline();

		operator VkPipeline() const { return m_graphicsPipeline; }
		VkPipeline pipeline() const { return m_graphicsPipeline; }

		void bind(VkCommandBuffer commandBuffer);

		static void defaultPipelineConfigInfo(Pipeline::Config& configInfo);
		static void enableAlphaBlending(Pipeline::Config& configInfo);

	private:
		void createGraphicsPipeline(const Pipeline::Config& configInfo);

		VulkanDevice& m_device;
		VkPipeline m_graphicsPipeline;
	};
}
