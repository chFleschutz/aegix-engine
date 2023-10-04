#pragma once

#include "device.h"
#include "swap_chain.h"
#include "window.h"

#include <cassert>
#include <memory>
#include <vector>

namespace vre
{
	class VreRenderer
	{
	public:
		VreRenderer(VreWindow& window, VreDevice& device);
		~VreRenderer();

		VreRenderer(const VreRenderer&) = delete;
		VreRenderer& operator=(const VreRenderer&) = delete;

		VkRenderPass swapChainRenderPass() const { return mVreSwapChain->renderPass(); }
		float aspectRatio() const { return mVreSwapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return mIsFrameStarted; }
		VkCommandBuffer currentCommandBuffer() const
		{
			assert(mIsFrameStarted && "Cannot get command buffer when frame not in progress");
			return mCommandBuffers[mCurrentFrameIndex];
		}

		int frameIndex() const 
		{
			assert(mIsFrameStarted && "Cannot get frame index when frame not in progress");
			return mCurrentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		VreWindow& mVreWindow;
		VreDevice& mVreDevice;
		std::unique_ptr<VreSwapChain> mVreSwapChain;
		std::vector<VkCommandBuffer> mCommandBuffers;

		uint32_t mCurrentImageIndex;
		int mCurrentFrameIndex;
		bool mIsFrameStarted = false;
	};

} // namespace vre
