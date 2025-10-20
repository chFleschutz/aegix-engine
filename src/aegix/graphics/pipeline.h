#pragma once

#include "graphics/vulkan/volk_include.h"

namespace Aegix::Graphics
{
	class Pipeline
	{
	public:
		enum class Flags
		{
			None = 0,
			MeshShader = 1 << 0,
		};

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
			Flags flags = Flags::None;
		};

		class GraphicsBuilder
		{
		public:
			GraphicsBuilder();
			~GraphicsBuilder();

			auto addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) -> GraphicsBuilder&;
			auto addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t size, uint32_t offset = 0) -> GraphicsBuilder&;

			auto addShaderStage(VkShaderStageFlagBits stage, const std::filesystem::path& shaderPath) -> GraphicsBuilder&;
			auto addShaderStages(VkShaderStageFlags stages, const std::filesystem::path& shaderPath) -> GraphicsBuilder&;
			auto addColorAttachment(VkFormat colorFormat, bool alphaBlending = false) -> GraphicsBuilder&;
			auto setDepthAttachment(VkFormat depthFormat) -> GraphicsBuilder&;
			auto setStencilFormat(VkFormat stencilFormat) -> GraphicsBuilder&;
			auto setDepthTest(bool enableDepthTest, bool writeDepth, VkCompareOp compareOp = VK_COMPARE_OP_LESS) -> GraphicsBuilder&;
			auto setCullMode(VkCullModeFlags cullMode) -> GraphicsBuilder&;
			auto setVertexBindingDescriptions(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions) -> GraphicsBuilder&;
			auto setVertexAttributeDescriptions(const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions) -> GraphicsBuilder&;
			auto addFlag(Flags flag) -> GraphicsBuilder&;

			auto buildUnique() -> std::unique_ptr<Pipeline>;
			auto build() -> Pipeline;

		private:
			void addShaderStage(VkShaderStageFlagBits stage, VkShaderModule shaderModule, const char* entryPoint);

			LayoutConfig m_layoutConfig;
			GraphicsConfig m_graphicsConfig;
			std::vector<VkShaderModule> m_shaderModules;
		};

		struct ComputeConfig
		{
			VkPipelineShaderStageCreateInfo shaderStage;
		};

		class ComputeBuilder
		{
		public:
			ComputeBuilder() = default;
			~ComputeBuilder();

			auto addDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) -> ComputeBuilder&;
			auto addPushConstantRange(VkShaderStageFlags stageFlags, uint32_t size) -> ComputeBuilder&;

			auto setShaderStage(const std::filesystem::path& shaderPath, const char* entry = "main") -> ComputeBuilder&;

			auto buildUnique() -> std::unique_ptr<Pipeline>;
			auto build() -> Pipeline;

		private:
			LayoutConfig m_layoutConfig;
			ComputeConfig m_computeConfig;
		};

		Pipeline() = default;
		Pipeline(const LayoutConfig& layoutConfig, const GraphicsConfig& graphicsConfig);
		Pipeline(const LayoutConfig& layoutConfig, const ComputeConfig& computeConfig);
		Pipeline(const Pipeline&) = delete;
		Pipeline(Pipeline&& other) noexcept;
		~Pipeline();

		auto operator=(const Pipeline&) -> Pipeline& = delete;
		auto operator=(Pipeline&& other) noexcept -> Pipeline&;

		operator VkPipeline() const { return m_pipeline; }

		[[nodiscard]] auto pipeline() const -> VkPipeline { return m_pipeline; }
		[[nodiscard]] auto layout() const -> VkPipelineLayout { return m_layout; }
		[[nodiscard]] auto bindPoint() const -> VkPipelineBindPoint { return m_bindPoint; }
		[[nodiscard]] auto hasFlag(Flags flag) const -> bool;

		void bind(VkCommandBuffer commandBuffer) const;
		void bindDescriptorSet(VkCommandBuffer cmd, uint32_t setIndex, VkDescriptorSet descriptorSet) const;
		void pushConstants(VkCommandBuffer cmd, VkShaderStageFlags stageFlags, const void* data, uint32_t size, uint32_t offset = 0) const;

		template<typename T>
		void pushConstants(VkCommandBuffer cmd, VkShaderStageFlags stageFlags, const T& data) const
		{
			pushConstants(cmd, stageFlags, &data, sizeof(T));
		}

		static void defaultGraphicsPipelineConfig(Pipeline::GraphicsConfig& configInfo);

	private:
		void createPipelineLayout(const LayoutConfig& config);
		void createGraphicsPipeline(const GraphicsConfig& config);
		void createComputePipeline(const ComputeConfig& config);
		void destroy();

		VkPipelineLayout m_layout = VK_NULL_HANDLE;
		VkPipeline m_pipeline = VK_NULL_HANDLE;
		VkPipelineBindPoint m_bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		Flags m_flags = Flags::None;
	};
}
