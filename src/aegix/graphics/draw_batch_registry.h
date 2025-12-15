#pragma once

#include "graphics/material/material_template.h"

#include "scene/scene.h"

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
		[[nodiscard]] auto staticInstanceCount() const -> uint32_t { return m_staticCount; }
		[[nodiscard]] auto dynamicInstanceCount() const -> uint32_t { return m_dynamicCount; }

		auto registerDrawBatch(std::shared_ptr<MaterialTemplate> mat) -> const DrawBatch&;
		void addInstance(uint32_t batchId);
		void removeInstance(uint32_t batchId);

		void sceneChanged(Scene::Scene& scene);

	private:
		void updateOffsets(uint32_t startBatchId = 0);

		void onMaterialCreated(entt::registry& reg, entt::entity e);
		void onMaterialRemoved(entt::registry& reg, entt::entity e);
		void onDynamicTagCreated(entt::registry& reg, entt::entity e);
		void onDynamicTagRemoved(entt::registry& reg, entt::entity e);

		std::vector<DrawBatch> m_batches;
		uint32_t m_staticCount{ 0 };
		uint32_t m_dynamicCount{ 0 };
		uint32_t m_totalCount{ 0 };
	};
}