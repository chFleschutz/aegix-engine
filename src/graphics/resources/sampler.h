#pragma once

#include "graphics/device.h"

namespace Aegix::Graphics
{
	class Sampler
	{
	public:
		struct Config
		{
			VkFilter magFilter = VK_FILTER_LINEAR;
			VkFilter minFilter = VK_FILTER_LINEAR;
			VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			bool anisotropy = true;
		};

		Sampler(VulkanDevice& device);
		Sampler(const Sampler&) = delete;
		Sampler(Sampler&& other) noexcept;
		~Sampler();

		auto operator=(const Sampler&)->Sampler & = delete;
		auto operator=(Sampler&& other) noexcept -> Sampler&;

		operator VkSampler() const { return m_sampler; }

		[[nodiscard]] auto sampler() const -> VkSampler { return m_sampler; }

		void create(const Config& config);
		void create(VkFilter magFilter, VkFilter minFilter, VkSamplerAddressMode addressMode, bool anisotropy);

	private:
		void destroy();

		VulkanDevice& m_device;
		VkSampler m_sampler = VK_NULL_HANDLE;
	};
}
