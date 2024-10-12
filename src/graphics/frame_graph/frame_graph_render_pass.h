#pragma once

#include "graphics/device.h"
#include "graphics/frame_info.h"
#include "graphics/texture.h"

namespace Aegix::Graphics
{
	struct Attachment
	{
		VkFormat format;
		VkClearValue clearValue;
		VkAttachmentLoadOp loadOp;
		std::shared_ptr<Texture> texture;
	};

	class FrameGraphRenderPass
	{
	public:
		FrameGraphRenderPass(VulkanDevice& device) : m_device{ device } {}
		FrameGraphRenderPass(const FrameGraphRenderPass&) = delete;
		virtual ~FrameGraphRenderPass() = default;

		virtual void execute(const FrameInfo& frameInfo) = 0;

		void setAttachments(std::vector<Attachment> attachments) { m_attachments = std::move(attachments); }

	protected:
		VulkanDevice& m_device;

		std::vector<Attachment> m_attachments;
		VkExtent2D m_renderArea;
	};
}