#include "pch.h"
#include "deletion_queue.h"

namespace Aegix::Graphics
{
	DeletionQueue::~DeletionQueue()
	{
		flushAll();
	}

	void DeletionQueue::schedule(std::function<void()>&& function)
	{
		m_pendingDeletions[m_currentFrameIndex].deletors.emplace_back(std::move(function));
	}

	void DeletionQueue::flush(uint32_t frameIndex)
	{
		m_currentFrameIndex = frameIndex;

		auto& deletors = m_pendingDeletions[frameIndex].deletors;
		for (auto& deleteFunc : deletors)
		{
			if (deleteFunc)
				deleteFunc();
		}
		deletors.clear();
	}

	void DeletionQueue::flushAll()
	{
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			flush(i);
		}
	}
}
