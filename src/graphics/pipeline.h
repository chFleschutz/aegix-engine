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
		struct GraphicsConfig
		{
			GraphicsConfig() = default;
			GraphicsConfig(const GraphicsConfig&) = delete;
			GraphicsConfig& operator=(const GraphicsConfig&) = delete;

			std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
			std::vector<VkPipelineShaderStageCreateInfo> shaderStges{};
			std::vector<VkFormat> colorAttachmentFormats{};
			VkPipelineRenderingCreateInfo renderingInfo{};
			VkPipelineViewportStateCreateInfo viewportInfo{};
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
			VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
			VkPipelineMultisampleStateCreateInfo multisampleInfo{};
			std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};
			VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
			VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
			std::vector<VkDynamicState> dynamicStateEnables{};
			VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
			VkPipelineLayout pipelineLayout = nullptr;
			uint32_t subpass = 0;
		};

		struct ComputeConfig
		{
			VkPipelineShaderStageCreateInfo shaderStage;
   			VkPipelineLayout pipelineLayout;
		};

		class GraphicsBuilder
		{
		public:
			GraphicsBuilder(VulkanDevice& device, VkPipelineLayout pipelineLayout);
			~GraphicsBuilder();

			GraphicsBuilder& addShaderStage(VkShaderStageFlagBits stage, const std::filesystem::path& shaderPath);
			GraphicsBuilder& addColorAttachment(VkFormat colorFormat, bool alphaBlending = false);
			GraphicsBuilder& setDepthAttachment(VkFormat depthFormat);
			GraphicsBuilder& setStencilFormat(VkFormat stencilFormat);
			GraphicsBuilder& setVertexBindingDescriptions(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions);
			GraphicsBuilder& setVertexAttributeDescriptions(const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);

			std::unique_ptr<Pipeline> build();

		private:
			VulkanDevice& m_device;
			Pipeline::GraphicsConfig m_config;
		};

		class ComputeBuilder
		{
		public:
			ComputeBuilder(VulkanDevice& device, VkPipelineLayout pipelineLayout);
			~ComputeBuilder();

			ComputeBuilder& setShaderStage(const std::filesystem::path& shaderPath);

			std::unique_ptr<Pipeline> build();

		private:
			VulkanDevice& m_device;
			ComputeConfig m_config;
		};

		Pipeline(VulkanDevice& device, const Pipeline::GraphicsConfig& config);
		Pipeline(VulkanDevice& device, const Pipeline::ComputeConfig& config);
		Pipeline(const Pipeline&) = delete;
		~Pipeline();

		Pipeline operator=(const Pipeline&) = delete;

		operator VkPipeline() const { return m_pipeline; }
		VkPipeline pipeline() const { return m_pipeline; }

		void bind(VkCommandBuffer commandBuffer) const;

		static void defaultGraphicsPipelineConfig(Pipeline::GraphicsConfig& configInfo);

	private:
		void createGraphicsPipeline(const Pipeline::GraphicsConfig& configInfo);

		VulkanDevice& m_device;
		VkPipeline m_pipeline = VK_NULL_HANDLE;
		VkPipelineBindPoint m_bindPoint;
	};
}
