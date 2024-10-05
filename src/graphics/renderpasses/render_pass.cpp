#include "render_pass.h"

namespace Aegix::Graphics
{
	RenderPass::RenderPass(VulkanDevice& device)
		: m_device{ device }
	{
		// TODO:
		//createRenderPass();
		//createFramebuffer();
	}

	void RenderPass::execute(FrameInfo& frameInfo)
	{
		// TODO: remove later
		m_renderPass = frameInfo.swapChainRenderPass;
		m_framebuffer = frameInfo.swapChainFramebuffer;
		m_renderArea = frameInfo.swapChainExtent;

		beginRenderPass(frameInfo.commandBuffer);
		render(frameInfo);
		endRenderPass(frameInfo.commandBuffer);
	}

	void RenderPass::createRenderPass()
	{
		// Subpass
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

		VkSubpassDependency dependency{};
		dependency.dstSubpass = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		// Attachments
		std::vector<VkAttachmentDescription> attachments;
		attachments.reserve(attachmentCount());

		// Color attachments
		std::vector<VkAttachmentReference> colorRefs;
		colorRefs.reserve(m_colorAttachments.size());
		for (size_t i = 0; i < m_colorAttachments.size(); i++)
		{
			auto& attachment = m_colorAttachments[i];

			VkAttachmentDescription attachmentDesc{};
			attachmentDesc.format = attachment.format;
			attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachments.emplace_back(attachmentDesc);

			VkAttachmentReference attachmentRef{};
			attachmentRef.attachment = static_cast<uint32_t>(i);
			attachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			colorRefs.emplace_back(attachmentRef);
		}

		subpass.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
		subpass.pColorAttachments = colorRefs.data();

		// Depth stencil attachment
		if (m_depthStencilAttachment.has_value())
		{
			VkAttachmentDescription depthStencilAttachment{};
			depthStencilAttachment.format = m_depthStencilAttachment->format;
			depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			attachments.emplace_back(depthStencilAttachment);

			VkAttachmentReference depthStencilRef{};
			depthStencilRef.attachment = static_cast<uint32_t>(colorRefs.size() - 1);
			depthStencilRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			subpass.pDepthStencilAttachment = &depthStencilRef;
		}

		// Render pass
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(m_device.device(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
			throw std::runtime_error("failed to create render pass!");
	}

	void RenderPass::createFramebuffer()
	{
		std::vector<VkImageView> attachments;
		attachments.reserve(attachmentCount());

		for (auto& attachment : m_colorAttachments)
		{
			assert(attachment.image && "Color attachment image not initialized");
			attachments.emplace_back(attachment.image->imageView());
		}

		if (m_depthStencilAttachment.has_value())
		{
			assert(m_depthStencilAttachment->image && "Depth/stencil attachment image not initialized");
			attachments.emplace_back(m_depthStencilAttachment->image->imageView()); 
		}

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_renderArea.width;
		framebufferInfo.height = m_renderArea.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_device.device(), &framebufferInfo, nullptr, &m_framebuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to create framebuffer!");
	}

	void RenderPass::beginRenderPass(VkCommandBuffer commandBuffer)
	{
		std::vector<VkClearValue> clearValues(attachmentCount());
		
		for (size_t i = 0; i < m_colorAttachments.size(); i++)
		{
			clearValues[i] = m_colorAttachments[i].clearValue;
		}

		if (m_depthStencilAttachment.has_value())
		{
			clearValues[m_colorAttachments.size()] = m_depthStencilAttachment->clearValue;
		}

		// TODO: remove later
		clearValues.resize(2);
		clearValues[0] = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1] = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_renderPass;
		renderPassInfo.framebuffer = m_framebuffer;
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = m_renderArea;
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_renderArea.width);
		viewport.height = static_cast<float>(m_renderArea.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_renderArea;

		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void RenderPass::endRenderPass(VkCommandBuffer commandBuffer)
	{
		vkCmdEndRenderPass(commandBuffer);
	}

}