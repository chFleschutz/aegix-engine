#pragma once

#include "graphics/frame_info.h"
#include "graphics/systems/render_system_collection.h"
#include "graphics/texture.h"

#include <memory>
#include <optional>
#include <vector>

namespace Aegix::Graphics
{
	class RenderPass
	{
	public:
		struct Attachment
		{
			VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
			VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
			VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
			std::shared_ptr<Texture> image;
		};

		struct SampledTexture
		{
			std::shared_ptr<Texture> texture;
		};

		/// @brief Builder class for creating RenderPass objects
		class Builder
		{
		public:
			Builder(VulkanDevice& device) : m_device{ device } {}

			Builder& addColorAttachment(const Attachment& attachment);
			Builder& addColorAttachment(std::shared_ptr<Texture> image, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f }, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
			Builder& setDepthStencilAttachment(const Attachment& attachment);
			Builder& addSampledTexture(const SampledTexture& texture);

			template<typename T, typename... Args>
			std::unique_ptr<RenderPass> build(Args&&... args)
			{ 
				static_assert(std::is_base_of_v<RenderPass, T>, "T must be a subclass of RenderPass");
				return std::make_unique<T>(*this, std::forward<Args>(args)...); 
			}

		private:
			VulkanDevice& m_device;
			std::vector<Attachment> m_colorAttachments;
			std::optional<Attachment> m_depthStencilAttachment;
			std::vector<SampledTexture> m_sampledTextures;

			friend class RenderPass;
		};


		explicit RenderPass(Builder& builder);
		RenderPass(const RenderPass&) = delete;
		RenderPass(RenderPass&&) = delete;
		virtual ~RenderPass() = default;

		size_t attachmentCount() const { return colorAttachmentCount() + (hasDepthStencilAttachment() ? 1 : 0); }
		size_t colorAttachmentCount() const { return m_colorAttachments.size(); }
		bool hasDepthStencilAttachment() const { return m_depthStencilAttachment.has_value(); }
		const Attachment& colorAttachment(size_t index) const { return m_colorAttachments[index]; }
		const Attachment& depthStencilAttachment() const { return m_depthStencilAttachment.value(); }

		/// @brief Renderes the render pass 
		virtual void execute(FrameInfo& frameInfo);

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
		std::vector<SampledTexture> m_sampledTextures;

		std::unique_ptr<DescriptorSetLayout> m_globalSetLayout;

		RenderSystemCollection m_renderSystemCollection;
	};
}