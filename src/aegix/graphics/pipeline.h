#pragma once

#include "graphics/device.h"

namespace Aegix::Graphics
{
	class Pipeline
	{
	public:
		struct LayoutConfig
		{
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
			std::vector<VkPushConstantRange> pushConstantRanges;
		};

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
			uint32_t subpass = 0;
		};

		class GraphicsBuilder
		{
		public:
			GraphicsBuilder(VulkanDevice& device);
			~GraphicsBuilder();

			auto addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) -> GraphicsBuilder&;
			auto addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t size, uint32_t offset = 0) -> GraphicsBuilder&;

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
			LayoutConfig m_layoutConfig;
			GraphicsConfig m_graphicsConfig;
		};

		struct ComputeConfig
		{
			VkPipelineShaderStageCreateInfo shaderStage;
		};

		class ComputeBuilder
		{
		public:
			ComputeBuilder(VulkanDevice& device);
			~ComputeBuilder();

			auto addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) -> ComputeBuilder&;
			auto addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t size, uint32_t offset = 0) -> ComputeBuilder&;

			auto setShaderStage(const std::filesystem::path& shaderPath) -> ComputeBuilder&;

			auto buildUnique() -> std::unique_ptr<Pipeline>;
			auto build() -> Pipeline;

		private:
			VulkanDevice& m_device;
			LayoutConfig m_layoutConfig;
			ComputeConfig m_computeConfig;
		};

		Pipeline(VulkanDevice& device);
		Pipeline(VulkanDevice& device, const LayoutConfig& layoutConfig, const GraphicsConfig& graphicsConfig);
		Pipeline(VulkanDevice& device, const LayoutConfig& layoutConfig, const ComputeConfig& computeConfig);
		Pipeline(const Pipeline&) = delete;
		Pipeline(Pipeline&& other) noexcept;
		~Pipeline();

		auto operator=(const Pipeline&) -> Pipeline& = delete;
		auto operator=(Pipeline&& other) noexcept -> Pipeline&;

		operator VkPipeline() const { return m_pipeline; }

		[[nodiscard]] auto pipeline() const -> VkPipeline { return m_pipeline; }
		[[nodiscard]] auto layout() const -> VkPipelineLayout { return m_Layout; }
		[[nodiscard]] auto bindPoint() const -> VkPipelineBindPoint { return m_bindPoint; }

		void bind(VkCommandBuffer commandBuffer) const;
		void bindDescriptorSet(VkCommandBuffer cmd, uint32_t setIndex, VkDescriptorSet descriptorSet) const;

		static void defaultGraphicsPipelineConfig(Pipeline::GraphicsConfig& configInfo);

	private:
		void createPipelineLayout(const LayoutConfig& config);
		void createGraphicsPipeline(const GraphicsConfig& config);
		void createComputePipeline(const ComputeConfig& config);
		void destroy();

		VulkanDevice& m_device;
		VkPipelineLayout m_Layout = VK_NULL_HANDLE;
		VkPipeline m_pipeline = VK_NULL_HANDLE;
		VkPipelineBindPoint m_bindPoint;
	};
}
