#pragma once

#include <vulkan/vulkan.h>

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
			VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			float maxLod = 0.0f;
			bool anisotropy = true;
		};

		Sampler() = default;
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

		VkSampler m_sampler = VK_NULL_HANDLE;
	};
}
