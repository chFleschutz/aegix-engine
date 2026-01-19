#pragma once

#include "graphics/vulkan/volk_include.h"

namespace Aegis::Graphics
{
	class Sampler
	{
	public:
		struct CreateInfo
		{
			static constexpr float USE_MIP_LEVELS = -1.0f;

			VkFilter magFilter = VK_FILTER_LINEAR;
			VkFilter minFilter = VK_FILTER_LINEAR;
			VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			float maxLod = USE_MIP_LEVELS;
			bool anisotropy = true;
		};

		Sampler() = default;
		explicit Sampler(const CreateInfo& config, uint32_t mipLevels = 1);
		Sampler(const Sampler&) = delete;
		Sampler(Sampler&& other) noexcept;
		~Sampler();

		auto operator=(const Sampler&)->Sampler & = delete;
		auto operator=(Sampler&& other) noexcept -> Sampler&;

		operator VkSampler() const { return m_sampler; }

		[[nodiscard]] auto sampler() const -> VkSampler { return m_sampler; }

	private:
		void destroy();

		VkSampler m_sampler = VK_NULL_HANDLE;
	};
}
