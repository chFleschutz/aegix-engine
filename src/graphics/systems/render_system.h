#pragma once

#include "graphics/buffer.h"
#include "graphics/descriptors.h"
#include "graphics/frame_info.h"
#include "graphics/pipeline.h"

namespace Aegix::Graphics
{
	/// @brief Forward declaration of RenderSystemRef. This is used to link a material to a render system
	/// @tparam T Type of the material
	/// @note See default_render_system.h for an example
	template<typename T>
	struct RenderSystemRef;


	/// @brief Base class for all render systems
	class RenderSystem
	{
	public:
		RenderSystem(VulkanDevice& device);
		RenderSystem(const RenderSystem&) = delete;
		RenderSystem& operator=(const RenderSystem&) = delete;
		virtual ~RenderSystem();

		DescriptorSetLayout& descriptorSetLayout() { return *m_descriptorSetLayout; }

		virtual void render(const FrameInfo& frameInfo) = 0;

	protected:
		void createPipelineLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts, uint32_t pushConstantSize);
		void createPipeline(VkRenderPass renderPass, const std::string& vertexShader, const std::string& fragmentShader);

		VulkanDevice& m_device;

		std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
		VkPipelineLayout m_pipelineLayout = nullptr;
		std::unique_ptr<Pipeline> m_pipeline;
	};
}
