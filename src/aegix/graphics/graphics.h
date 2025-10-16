#pragma once

#include "graphics/globals.h"
#include "graphics/renderer.h"
#include "graphics/vulkan/vulkan_context.h"

namespace Aegix::Graphics
{
	class Graphics
	{
	public:
		Graphics(Core::Window& window)
			: m_context{ VulkanContext::initialize(window) }, m_renderer{ window }
		{
		}

		Graphics(const Graphics&) = delete;
		Graphics(Graphics&&) = delete;
		~Graphics() = default;

		auto operator=(const Graphics&) -> Graphics& = delete;
		auto operator=(Graphics&&) -> Graphics& = delete;

		[[nodiscard]] auto context() -> VulkanContext& { return m_context; }
		[[nodiscard]] auto renderer() -> Renderer& { return m_renderer; }

	private:
		VulkanContext& m_context;
		Renderer m_renderer;
	};
}
