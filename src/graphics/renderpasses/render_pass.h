#pragma once

#include "graphics/frame_info.h"
#include "graphics/systems/render_system_collection.h"
#include "graphics/texture.h"

#include <memory>
#include <optional>
#include <vector>

namespace Aegix::Graphics
{
	struct Attachment
	{
		enum class Type
		{
			Color,
			DepthStencil,
		};

		VkFormat format;
		Type type;
		VkClearValue clearValue;
		std::shared_ptr<Texture> image;
	};

	class RenderPass
	{
	public:
		RenderPass(VulkanDevice& device);
		virtual ~RenderPass() = default;

		size_t attachmentCount() const { return colorAttachmentCount() + (hasDepthStencilAttachment() ? 1 : 0); }
		size_t colorAttachmentCount() const { return m_colorAttachments.size(); }
		bool hasDepthStencilAttachment() const { return m_depthStencilAttachment.has_value(); }
		const Attachment& colorAttachment(size_t index) const { return m_colorAttachments[index]; }
		const Attachment& depthStencilAttachment() const { return m_depthStencilAttachment.value(); }

		/// @brief Renderes the render pass 
		void execute(FrameInfo& frameInfo);

		template<typename T>
		RenderSystem& addRenderSystem(VulkanDevice& device, VkRenderPass renderpass)
		{
			assert(m_globalSetLayout && "Global descriptor set layout not initialized");
			return m_renderSystemCollection.addRenderSystem<T>(device, renderpass, m_globalSetLayout->descriptorSetLayout());
		}

	protected:
		virtual void createRenderPass();
		virtual void createFramebuffer();

		virtual void beginRenderPass(VkCommandBuffer commandBuffer);
		virtual void render(FrameInfo& frameInfo) = 0;
		virtual void endRenderPass(VkCommandBuffer commandBuffer);

		VulkanDevice& m_device;

		VkRenderPass m_renderPass = VK_NULL_HANDLE;
		VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
		VkExtent2D m_renderArea{};

		std::vector<Attachment> m_colorAttachments;
		std::optional<Attachment> m_depthStencilAttachment;

		RenderSystemCollection m_renderSystemCollection;
		std::unique_ptr<DescriptorSetLayout> m_globalSetLayout;
	};
}