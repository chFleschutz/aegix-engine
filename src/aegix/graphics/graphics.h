#pragma once

#include "graphics/device.h"
#include "graphics/globals.h"
#include "graphics/renderer.h"
#include "graphics/vulkan_context.h"

namespace Aegix::Graphics
{
	class Graphics
	{
	public:
		Graphics(Core::Window& window)
			: m_context{ VulkanContext::initialize(window) }, m_renderer{ window, m_context.device() }
		{
		}

		Graphics(const Graphics&) = delete;
		Graphics(Graphics&&) = delete;
		~Graphics() = default;

		auto operator=(const Graphics&) -> Graphics& = delete;
		auto operator=(Graphics&&) -> Graphics& = delete;

		[[nodiscard]] auto device() -> VulkanDevice& { return m_context.device(); }
		[[nodiscard]] auto renderer() -> Renderer& { return m_renderer; }

	private:
		VulkanContext& m_context;
		Renderer m_renderer;
	};
}
