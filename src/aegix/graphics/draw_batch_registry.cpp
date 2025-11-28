#include "pch.h"
#include "draw_batch_registry.h"

namespace Aegix::Graphics
{
	auto DrawBatchRegistry::registerDrawBatch(std::shared_ptr<MaterialTemplate> mat) -> const DrawBatch&
	{
		auto it = std::find_if(m_batches.begin(), m_batches.end(),
			[&mat](const DrawBatch& batch) {
				return batch.materialTemplate == mat;
			});
		if (it != m_batches.end())
			return *it;

		uint32_t nextId = static_cast<uint32_t>(m_batches.size());
		m_batches.emplace_back(nextId, m_totalCount, 0, std::move(mat));
		return m_batches.back();
	}

	void DrawBatchRegistry::addInstance(uint32_t batchId)
	{
		AGX_ASSERT_X(isValid(batchId), "Invalid batch ID");

		m_batches[batchId].instanceCount++;
		updateOffsets(batchId);
	}

	void DrawBatchRegistry::removeInstance(uint32_t batchId)
	{
		AGX_ASSERT_X(isValid(batchId), "Invalid batch ID");
		AGX_ASSERT_X(m_batches[batchId].instanceCount > 0, "Batch count is already zero");

		m_batches[batchId].instanceCount--;
		updateOffsets(batchId);
	}

	void DrawBatchRegistry::updateOffsets(uint32_t startBatchId)
	{
		if (!isValid(startBatchId))
			return;

		uint32_t baseOffset = m_batches[startBatchId].firstInstance + m_batches[startBatchId].instanceCount;
		for (uint32_t i = startBatchId + 1; i < static_cast<uint32_t>(m_batches.size()); i++)
		{
			m_batches[i].firstInstance = baseOffset;
			baseOffset += m_batches[i].instanceCount;
		}
		m_totalCount = baseOffset;
	}
}
