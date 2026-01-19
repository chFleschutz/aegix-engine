#pragma once

#include "graphics/globals.h"

namespace Aegis::Graphics
{
	/// @brief Manages the deletion of vulkan objects to ensure they are not deleted while in use
	/// @note Deletion is deferred for SwapChain::MAX_FRAMES_IN_FLIGHT frames 
	class DeletionQueue
	{
	public:
		DeletionQueue() = default;
		DeletionQueue(const DeletionQueue&) = delete;
		DeletionQueue(DeletionQueue&&) = delete;
		~DeletionQueue();

		DeletionQueue& operator=(const DeletionQueue&) = delete;
		DeletionQueue& operator=(DeletionQueue&&) = delete;

		void schedule(std::function<void()>&& function);
		void flush(uint32_t frameIndex);
		void flushAll();

	private:
		struct DeletorQueue
		{
			std::vector<std::function<void()>> deletors;
		};

		std::array<DeletorQueue, MAX_FRAMES_IN_FLIGHT> m_pendingDeletions;
		uint32_t m_currentFrameIndex{ 0 };
	};
}