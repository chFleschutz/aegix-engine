#include "pch.h"
#include "sampler.h"

#include "graphics/vulkan/vulkan_context.h"
#include "graphics/vulkan/vulkan_tools.h"

namespace Aegix::Graphics
{
	Sampler::Sampler(const CreateInfo& config, uint32_t mipLevels)
	{
		VkSamplerCreateInfo samplerInfo{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = config.magFilter,
			.minFilter = config.minFilter,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = config.addressMode,
			.addressModeV = config.addressMode,
			.addressModeW = config.addressMode,
			.mipLodBias = 0.0f,
			.anisotropyEnable = config.anisotropy,
			.maxAnisotropy = VulkanContext::device().properties().limits.maxSamplerAnisotropy,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_ALWAYS,
			.minLod = 0.0f,
			.maxLod = config.maxLod < 0.0f ? static_cast<float>(mipLevels - 1) : config.maxLod,
			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
			.unnormalizedCoordinates = VK_FALSE,
		};
		VK_CHECK(vkCreateSampler(VulkanContext::device(), &samplerInfo, nullptr, &m_sampler));
	}

	Sampler::Sampler(Sampler&& other) noexcept
		: m_sampler{ other.m_sampler }
	{
		other.m_sampler = VK_NULL_HANDLE;
	}

	Sampler::~Sampler()
	{
		destroy();
	}

	auto Sampler::operator=(Sampler&& other) noexcept -> Sampler&
	{
		if (this != &other)
		{
			destroy();
			m_sampler = other.m_sampler;
			other.m_sampler = VK_NULL_HANDLE;
		}
		return *this;
	}

	//void Sampler::create(const CreateInfo& config)
	//{
	//	destroy();

	//	VkSamplerCreateInfo samplerInfo{};
	//	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	//	samplerInfo.magFilter = config.magFilter;
	//	samplerInfo.minFilter = config.minFilter;
	//	samplerInfo.addressModeU = config.addressMode;
	//	samplerInfo.addressModeV = config.addressMode;
	//	samplerInfo.addressModeW = config.addressMode;
	//	samplerInfo.anisotropyEnable = config.anisotropy;
	//	samplerInfo.maxAnisotropy = VulkanContext::device().properties().limits.maxSamplerAnisotropy;
	//	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	//	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	//	samplerInfo.compareEnable = VK_FALSE;
	//	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	//	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	//	samplerInfo.mipLodBias = 0.0f;
	//	samplerInfo.minLod = 0.0f;
	//	samplerInfo.maxLod = config.maxLod;

	//	VK_CHECK(vkCreateSampler(VulkanContext::device(), &samplerInfo, nullptr, &m_sampler));
	//}

	//void Sampler::create(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode, bool anisotropy)
	//{
	//	CreateInfo config{
	//		.magFilter = magFilter,
	//		.minFilter = minFilter,
	//		.addressMode = addressMode,
	//		.anisotropy = anisotropy,
	//	};
	//	create(config);
	//}

	void Sampler::destroy()
	{
		VulkanContext::destroy(m_sampler);
		m_sampler = VK_NULL_HANDLE;
	}
}
