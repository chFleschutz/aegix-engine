#pragma once

#include "graphics/renderpasses/render_pass.h"
#include "graphics/swap_chain.h"

namespace Aegix::Graphics
{
	class SwapChainBlitPass : public RenderPass
	{
	public:
		SwapChainBlitPass(Builder& builder, SwapChain& swapChain);

	protected:
		void render(FrameInfo& frameInfo) override;

	private:
		SwapChain& m_swapChain;
	};
}