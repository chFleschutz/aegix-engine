#include "pch.h"

#include "sampler.h"

#include "graphics/vulkan_tools.h"

namespace Aegix::Graphics
{
	Sampler::Sampler(VulkanDevice& device)
		: m_device{ device }
	{
	}

	Sampler::Sampler(Sampler&& other) noexcept
		: m_device{ other.m_device }, m_sampler{ other.m_sampler }
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

	void Sampler::create(const Config& config)
	{
		destroy();

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = config.magFilter;
		samplerInfo.minFilter = config.minFilter;
		samplerInfo.addressModeU = config.addressMode;
		samplerInfo.addressModeV = config.addressMode;
		samplerInfo.addressModeW = config.addressMode;
		samplerInfo.anisotropyEnable = config.anisotropy;
		samplerInfo.maxAnisotropy = m_device.properties().limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = config.maxLod;

		VK_CHECK(vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &m_sampler));
	}

	void Sampler::create(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode, bool anisotropy)
	{
		Config config{
			.magFilter = magFilter,
			.minFilter = minFilter,
			.addressMode = addressMode,
			.anisotropy = anisotropy,
		};
		create(config);
	}

	void Sampler::destroy()
	{
		m_device.destroySampler(m_sampler);
		m_sampler = VK_NULL_HANDLE;
	}
}
