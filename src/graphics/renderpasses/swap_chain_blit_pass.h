#pragma once

#include "graphics/renderpasses/render_pass.h"
#include "graphics/swap_chain.h"

namespace Aegix::Graphics
{
	/// @brief A render pass that blits the color attachment to the swap chain
	class SwapChainBlitPass : public RenderPass
	{
	public:
		SwapChainBlitPass(Builder& builder, SwapChain& swapChain);

		virtual void execute(FrameInfo& frameInfo) override;

	protected:
		void render(FrameInfo& frameInfo) override;

	private:
		SwapChain& m_swapChain;
	};
}