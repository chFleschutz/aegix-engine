#include "pch.h"
#include "frame_graph_resources.h"

namespace Aegix::Graphics
{
	auto FGResource::toAccessInfo(Usage usage) -> AccessInfo
	{
		switch (usage)
		{
		default:
		case Usage::None:
			return AccessInfo{
				.stage = 0,
				.access = 0,
				.layout = VK_IMAGE_LAYOUT_UNDEFINED
			};
		case Usage::ColorAttachment:
			return AccessInfo{
				.stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};
		case Usage::DepthStencilAttachment:
			return AccessInfo{
				.stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				.access = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			};
		case Usage::FragmentReadSampled:
			return AccessInfo{
				.stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.access = VK_ACCESS_SHADER_READ_BIT,
				.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
		case Usage::ComputeReadStorage:
			return AccessInfo{
				.stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				.access = VK_ACCESS_SHADER_READ_BIT,
				.layout = VK_IMAGE_LAYOUT_GENERAL
			};
		case Usage::ComputeWriteStorage:
			return AccessInfo{
				.stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				.access = VK_ACCESS_SHADER_WRITE_BIT,
				.layout = VK_IMAGE_LAYOUT_GENERAL
			};
		case Usage::ComputeReadSampled:
			return AccessInfo{
				.stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				.access = VK_ACCESS_SHADER_READ_BIT,
				.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
		case Usage::TransferSrc:
			return AccessInfo{
				.stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
				.access = VK_ACCESS_TRANSFER_READ_BIT,
				.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
			};
		case Usage::TransferDst:
			return AccessInfo{
				.stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
				.access = VK_ACCESS_TRANSFER_WRITE_BIT,
				.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			};
		case Usage::Present:
			return AccessInfo{
				.stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				.access = 0,
				.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			};
		}
	}

	auto FGResource::toImageUsage(Usage usage) -> VkImageUsageFlags
	{
		switch (usage)
		{
		case Usage::ColorAttachment:
			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		case Usage::DepthStencilAttachment:
			return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		case Usage::FragmentReadSampled:
			return VK_IMAGE_USAGE_SAMPLED_BIT;
		case Usage::ComputeReadStorage:
		case Usage::ComputeWriteStorage:
			return VK_IMAGE_USAGE_STORAGE_BIT;
		case Usage::ComputeReadSampled:
			return VK_IMAGE_USAGE_SAMPLED_BIT;
		case Usage::TransferSrc:
			return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		case Usage::TransferDst:
			return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case Usage::Present:
			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Typically also a color attachment
		default:
			return 0;
		}
	}

	auto FGResource::toBufferUsage(Usage usage) -> VkBufferUsageFlags
	{
		switch (usage)
		{
		case Usage::ComputeReadStorage:
		case Usage::ComputeWriteStorage:
			return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		case Usage::TransferSrc:
			return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		case Usage::TransferDst:
			return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		default:
			return 0;
		}
	}
}
