#pragma once

#include "graphics/device.h"

namespace Aegix::Graphics
{
	class PipelineLayout
	{
	public:
		struct Config
		{
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
			std::vector<VkPushConstantRange> pushConstantRanges;
		};

		class Builder
		{
		public:
			Builder(VulkanDevice& device);
			~Builder() = default;

			auto addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) -> Builder&;
			auto addPushConstantRange(VkPushConstantRange pushConstantRange) -> Builder&;
			auto addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t size, uint32_t offset = 0) -> Builder&;
			auto buildUnique() -> std::unique_ptr<PipelineLayout>;
			auto build() -> PipelineLayout;

		private:
			VulkanDevice& m_device;
			Config m_config;
		};

		PipelineLayout(VulkanDevice& device);
		PipelineLayout(VulkanDevice& device, const Config& config);
		PipelineLayout(const PipelineLayout&) = delete;
		PipelineLayout(PipelineLayout&& other) noexcept;
		~PipelineLayout();

		auto operator=(const PipelineLayout&) -> PipelineLayout = delete;
		auto operator=(PipelineLayout&& other) noexcept -> PipelineLayout&;

		operator VkPipelineLayout() const { return m_pipelineLayout; }
		auto pipelineLayout() const -> VkPipelineLayout { return m_pipelineLayout; }

	private:
		void destroy();

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

		class GraphicsBuilder
		{
		public:
			GraphicsBuilder(VulkanDevice& device, VkPipelineLayout pipelineLayout);
			~GraphicsBuilder();

			auto addShaderStage(VkShaderStageFlagBits stage, const std::filesystem::path& shaderPath) -> GraphicsBuilder&;
			auto addColorAttachment(VkFormat colorFormat, bool alphaBlending = false) -> GraphicsBuilder&;
			auto setDepthAttachment(VkFormat depthFormat) -> GraphicsBuilder&;
			auto setStencilFormat(VkFormat stencilFormat) -> GraphicsBuilder&;
			auto setDepthTest(bool enableDepthTest, bool writeDepth, VkCompareOp compareOp = VK_COMPARE_OP_LESS) -> GraphicsBuilder&;
			auto setCullMode(VkCullModeFlags cullMode) -> GraphicsBuilder&;
			auto setVertexBindingDescriptions(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions) -> GraphicsBuilder&;
			auto setVertexAttributeDescriptions(const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions) -> GraphicsBuilder&;
			auto buildUnique() -> std::unique_ptr<Pipeline>;
			auto build() -> Pipeline;

		private:
			VulkanDevice& m_device;
			Pipeline::GraphicsConfig m_config;
		};

		struct ComputeConfig
		{
			VkPipelineShaderStageCreateInfo shaderStage;
			VkPipelineLayout pipelineLayout;
		};

		class ComputeBuilder
		{
		public:
			ComputeBuilder(VulkanDevice& device, VkPipelineLayout pipelineLayout);
			~ComputeBuilder();

			auto setShaderStage(const std::filesystem::path& shaderPath) -> ComputeBuilder&;
			auto buildUnique() -> std::unique_ptr<Pipeline>;
			auto build() -> Pipeline;

		private:
			VulkanDevice& m_device;
			ComputeConfig m_config;
		};

		Pipeline(VulkanDevice& device);
		Pipeline(VulkanDevice& device, const Pipeline::GraphicsConfig& config);
		Pipeline(VulkanDevice& device, const Pipeline::ComputeConfig& config);
		Pipeline(const Pipeline&) = delete;
		Pipeline(Pipeline&& other) noexcept;
		~Pipeline();

		auto operator=(const Pipeline&) -> Pipeline& = delete;
		auto operator=(Pipeline&& other) noexcept -> Pipeline&;

		operator VkPipeline() const { return m_pipeline; }
		auto pipeline() const -> VkPipeline { return m_pipeline; }

		void bind(VkCommandBuffer commandBuffer) const;

		static void defaultGraphicsPipelineConfig(Pipeline::GraphicsConfig& configInfo);

	private:
		void createGraphicsPipeline(const Pipeline::GraphicsConfig& configInfo);
		void destroy();

		VulkanDevice& m_device;
		VkPipeline m_pipeline = VK_NULL_HANDLE;
		VkPipelineBindPoint m_bindPoint;
	};
}
