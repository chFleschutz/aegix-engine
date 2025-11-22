#include "pch.h"
#include "draw_batch_registry.h"

namespace Aegix::Graphics
{
	auto DrawBatchRegistry::registerDrawBatch() -> const DrawBatch&
	{
		uint32_t nextId = static_cast<uint32_t>(m_batches.size());
		m_batches.emplace_back(nextId, m_totalCount, 0);
		return m_batches.back();
	}

	void DrawBatchRegistry::incrementBatchCount(uint32_t batchId)
	{
		AGX_ASSERT_X(isValid(batchId), "Invalid batch ID");

		m_batches[batchId].count++;
		updateOffsets(batchId);
	}

	void DrawBatchRegistry::decrementBatchCount(uint32_t batchId)
	{
		AGX_ASSERT_X(isValid(batchId), "Invalid batch ID");
		AGX_ASSERT_X(m_batches[batchId].count > 0, "Batch count is already zero");

		m_batches[batchId].count--;
		updateOffsets(batchId);
	}

	void DrawBatchRegistry::updateOffsets(uint32_t startBatchId)
	{
		if (!isValid(startBatchId))
			return;

		uint32_t baseOffset = m_batches[startBatchId].offset + m_batches[startBatchId].count;
		for (uint32_t i = startBatchId + 1; i < static_cast<uint32_t>(m_batches.size()); i++)
		{
			m_batches[i].offset = baseOffset;
			baseOffset += m_batches[i].count;
		}
		m_totalCount = baseOffset;
	}
}
