#pragma once

#include "graphics/material/material_template.h"

namespace Aegix::Graphics
{
	struct DrawBatch
	{
		uint32_t id;
		uint32_t firstInstance;
		uint32_t instanceCount;
		std::shared_ptr<MaterialTemplate> materialTemplate;
	};

	class DrawBatchRegistry
	{
	public:
		static constexpr uint32_t MAX_DRAW_BATCHES = 256;

		DrawBatchRegistry() = default;
		~DrawBatchRegistry() = default;

		[[nodiscard]] auto isValid(uint32_t batchId) const -> bool { return batchId < static_cast<uint32_t>(m_batches.size()); }
		[[nodiscard]] auto batches() const -> const std::vector<DrawBatch>& { return m_batches; }
		[[nodiscard]] auto batch(uint32_t indexindex) const -> const DrawBatch& { return m_batches[indexindex]; }
		[[nodiscard]] auto batchCount() const -> uint32_t { return static_cast<uint32_t>(m_batches.size()); }
		[[nodiscard]] auto instanceCount() const -> uint32_t { return m_totalCount; }

		auto registerDrawBatch(std::shared_ptr<MaterialTemplate> mat) -> const DrawBatch&;
		void addInstance(uint32_t batchId);
		void removeInstance(uint32_t batchId);

	private:
		void updateOffsets(uint32_t startBatchId = 0);

		std::vector<DrawBatch> m_batches;
		uint32_t m_totalCount{ 0 };
	};
}