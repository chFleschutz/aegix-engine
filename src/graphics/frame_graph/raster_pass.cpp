#include "raster_pass.h"

#include "graphics/vulkan_tools.h"

namespace Aegix::Graphics
{
	void RasterPass::createRenderPass()
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
		size_t depthAttachmentIndex = -1;

		std::vector<VkAttachmentDescription> attachmentDescriptions;
		attachmentDescriptions.reserve(m_attachments.size());

		std::vector<VkAttachmentReference> colorRefs;
		colorRefs.reserve(m_attachments.size()); // Potentially one to many but thats fine

		// Color attachments
		for (size_t i = 0; i < m_attachments.size(); i++)
		{
			auto& attachment = m_attachments[i];

			// Skip depth attachments
			if (Tools::isDepthFormat(m_attachments[i].format))
			{
				assert(depthAttachmentIndex == -1 && "Only one depth attachment is allowed");
				depthAttachmentIndex = i;
				continue;
			}

			VkAttachmentDescription attachmentDesc{};
			attachmentDesc.format = attachment.format;
			attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDesc.loadOp = attachment.loadOp;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachmentDescriptions.emplace_back(attachmentDesc);

			VkAttachmentReference attachmentRef{};
			attachmentRef.attachment = static_cast<uint32_t>(i);
			attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorRefs.emplace_back(attachmentRef);
		}

		subpass.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
		subpass.pColorAttachments = colorRefs.data();

		// Depth attachment
		if (depthAttachmentIndex != -1)
		{
			auto& attachment = m_attachments[depthAttachmentIndex];

			VkAttachmentDescription attachmentDesc{};
			attachmentDesc.format = attachment.format;
			attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
			attachmentDesc.loadOp = attachment.loadOp;
			attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
			attachmentDescriptions.emplace_back(attachmentDesc);

			VkAttachmentReference depthStencilRef{};
			depthStencilRef.attachment = static_cast<uint32_t>(colorRefs.size() - 1);
			depthStencilRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			subpass.pDepthStencilAttachment = &depthStencilRef;
		}

		// Render pass
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		CHECK_VK_RESULT(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass))
	}

	void RasterPass::createFramebuffer()
	{
		std::vector<VkImageView> attachments;
		attachments.reserve(m_attachments.size());

		size_t depthAttachmentIndex = -1;
		for (size_t i = 0; i < m_attachments.size(); i++)
		{
			auto& attachment = m_attachments[i];
			if (Tools::isDepthFormat(attachment.format))
			{
				assert(depthAttachmentIndex == -1 && "Only one depth attachment is allowed");
				depthAttachmentIndex = i;
				continue;
			}

			assert(attachment.texture && "Color attachment image not initialized");
			attachments.emplace_back(attachment.texture->imageView());
		}

		if (depthAttachmentIndex != -1)
		{
			auto& attachment = m_attachments[depthAttachmentIndex];
			assert(attachment.texture && "Depth/stencil attachment image not initialized");
			attachments.emplace_back(attachment.texture->imageView());
		}

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = m_renderArea.width;
		framebufferInfo.height = m_renderArea.height;
		framebufferInfo.layers = 1;

		CHECK_VK_RESULT(vkCreateFramebuffer(m_device.device(), &framebufferInfo, nullptr, &m_framebuffer));
	}
}