#include "raster_pass.h"

#include "graphics/vulkan_tools.h"

namespace Aegix::Graphics
{
	void RasterPass::create()
	{
		createRenderPass();
		createFramebuffer();
		createClearValues();
	}

	void RasterPass::execute(const FrameInfo& frameInfo)
	{
		beginRenderPass(frameInfo.commandBuffer);

		FrameGraphPass::execute(frameInfo);

		endRenderPass(frameInfo.commandBuffer);
	}

	void RasterPass::createRenderArea()
	{
		for (const auto& [resource, usage] : m_outputs)
		{
			if (usage != ResourceUsage::ColorAttachment && usage != ResourceUsage::DepthAttachment)
				continue;

			assert(resource->texture && "Attachment image not initialized");
			m_renderArea = resource->texture->extent();
		}
	}

	void RasterPass::createRenderPass()
	{
		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkAttachmentReference> colorRefs;
		VkAttachmentReference depthRef{};

		// TODO: Only works for output resources (Read-only resources are not considered)
		for (const auto& [resource, usage] : m_outputs)
		{
			if (usage != ResourceUsage::ColorAttachment && usage != ResourceUsage::DepthAttachment)
				continue;

			assert(resource->texture && "Color attachment image not initialized");

			VkAttachmentDescription attachmentDesc{};
			attachmentDesc.format = resource->texture->format();
			attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

			if (usage == ResourceUsage::ColorAttachment)
			{
				attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

				VkAttachmentReference attachmentRef{};
				attachmentRef.attachment = static_cast<uint32_t>(attachments.size()); // Size equals next index
				attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorRefs.emplace_back(attachmentRef);
			}
			else if (usage == ResourceUsage::DepthAttachment)
			{
				attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

				assert(depthRef.attachment == VK_ATTACHMENT_UNUSED && "Only one depth attachment is allowed");
				depthRef.attachment = static_cast<uint32_t>(attachments.size()); // Size equals next index
				depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}

			attachments.emplace_back(attachmentDesc);
		}

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
		subpass.pColorAttachments = colorRefs.data();

		if (depthRef.attachment != VK_ATTACHMENT_UNUSED)
			subpass.pDepthStencilAttachment = &depthRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		CHECK_VK_RESULT(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass));
	}

	void RasterPass::createFramebuffer()
	{
		std::vector<VkImageView> attachments{};

		for (const auto& [resource, usage] : m_outputs)
		{
			if (usage != ResourceUsage::ColorAttachment && usage != ResourceUsage::DepthAttachment)
				continue;

			assert(resource->texture && "Attachment image not initialized");

			attachments.emplace_back(resource->texture->imageView());
		}

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_renderArea.width;
		framebufferInfo.height = m_renderArea.height;
		framebufferInfo.layers = 1;

		CHECK_VK_RESULT(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_framebuffer));
	}

	void RasterPass::createClearValues()
	{
		for (const auto& [resource, usage] : m_outputs)
		{
			if (usage == ResourceUsage::ColorAttachment)
			{
				VkClearValue clearValue{};
				clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
				m_clearValues.emplace_back(clearValue);
			}
			else if (usage == ResourceUsage::DepthAttachment)
			{
				VkClearValue clearValue{};
				clearValue.depthStencil = { 1.0f, 0 };
				m_clearValues.emplace_back(clearValue);
			}
		}
	}

	void RasterPass::beginRenderPass(VkCommandBuffer commandBuffer)
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_renderPass;
		renderPassInfo.framebuffer = m_framebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_renderArea;
		renderPassInfo.clearValueCount = static_cast<uint32_t>(m_clearValues.size());
		renderPassInfo.pClearValues = m_clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void RasterPass::endRenderPass(VkCommandBuffer commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
	}
}