#pragma once

#include "renderer/device.h"
#include "renderer/swap_chain.h"
#include "window.h"

#include <cassert>
#include <memory>
#include <vector>

namespace vre
{
	class Renderer
	{
	public:
		Renderer(Window& window, VulkanDevice& device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

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

		Window& mVreWindow;
		VulkanDevice& mVreDevice;
		std::unique_ptr<VreSwapChain> mVreSwapChain;
		std::vector<VkCommandBuffer> mCommandBuffers;

		uint32_t mCurrentImageIndex;
		int mCurrentFrameIndex;
		bool mIsFrameStarted = false;
	};

} // namespace vre
