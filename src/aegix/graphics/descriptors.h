#pragma once

#include "graphics/globals.h"
#include "graphics/resources/texture.h"

namespace Aegix::Graphics
{
	class DescriptorSetLayout
	{
		friend class DescriptorWriter;

	public:
		struct CreateInfo
		{
			std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
			VkDescriptorBindingFlags bindingFlags;
			VkDescriptorSetLayoutCreateFlags flags{ 0 };
		};

		class Builder
		{
		public:
			Builder() = default;

			auto addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags,
				uint32_t count = 1) -> Builder&;
			auto setBindingFlags(VkDescriptorBindingFlags flags) -> Builder&;
			auto setFlags(VkDescriptorSetLayoutCreateFlags flags) -> Builder&;
			auto buildUnique() -> std::unique_ptr<DescriptorSetLayout>;
			auto build() -> DescriptorSetLayout;

		private:
			CreateInfo m_createInfo{};
		};

		DescriptorSetLayout(CreateInfo& createInfo);
		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
		~DescriptorSetLayout();

		auto operator=(const DescriptorSetLayout&) -> DescriptorSetLayout& = delete;
		auto operator=(DescriptorSetLayout&& other) noexcept -> DescriptorSetLayout&;

		operator VkDescriptorSetLayout() const { return m_descriptorSetLayout; }
		auto descriptorSetLayout() const -> VkDescriptorSetLayout { return m_descriptorSetLayout; }

		auto allocateDescriptorSet() const -> VkDescriptorSet;

	private:
		VkDescriptorSetLayout m_descriptorSetLayout{ VK_NULL_HANDLE };
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings;
	};



	class DescriptorPool
	{
		friend class DescriptorWriter;

	public:
		class Builder
		{
		public:
			Builder() = default;

			auto addPoolSize(VkDescriptorType descriptorType, uint32_t count) -> Builder&;
			auto setPoolFlags(VkDescriptorPoolCreateFlags flags) -> Builder&;
			auto setMaxSets(uint32_t count) -> Builder&;
			auto build() const -> DescriptorPool;
			auto buildUnique() const -> std::unique_ptr<DescriptorPool>;

		private:
			std::vector<VkDescriptorPoolSize> m_poolSizes{};
			uint32_t m_maxSets = 1000;
			VkDescriptorPoolCreateFlags m_poolFlags = 0;
		};

		DescriptorPool() = default;
		DescriptorPool(uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags,
			const std::vector<VkDescriptorPoolSize>& poolSizes);
		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&& other) noexcept;
		~DescriptorPool();

		auto operator=(const DescriptorPool&) -> DescriptorPool& = delete;
		auto operator=(DescriptorPool&& other) noexcept -> DescriptorPool&;

		operator VkDescriptorPool() const { return m_descriptorPool; }
		auto descriptorPool() const -> VkDescriptorPool { return m_descriptorPool; }

		void allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;
		void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;
		void resetPool();

	private:
		VkDescriptorPool m_descriptorPool{ VK_NULL_HANDLE };
	};



	class DescriptorWriter
	{
	public:
		DescriptorWriter(DescriptorSetLayout& setLayout);
		DescriptorWriter(const DescriptorWriter&) = delete;
		DescriptorWriter(DescriptorWriter&&) = default;
		~DescriptorWriter() = default;

		auto operator=(const DescriptorWriter&) -> DescriptorWriter& = delete;
		auto operator=(DescriptorWriter&&) -> DescriptorWriter& = default;

		auto writeImage(uint32_t binding, const Texture& texture) -> DescriptorWriter&;
		auto writeImage(uint32_t binding, const Texture& texture, VkImageLayout layoutOverride) -> DescriptorWriter&;
		auto writeImage(uint32_t binding, VkDescriptorImageInfo imageInfo) -> DescriptorWriter&;

		auto writeBuffer(uint32_t binding, const Buffer& buffer) -> DescriptorWriter&;
		auto writeBuffer(uint32_t binding, const Buffer& buffer, uint32_t index) -> DescriptorWriter&;
		auto writeBuffer(uint32_t binding, VkDescriptorBufferInfo bufferInfo) -> DescriptorWriter&;

		void update(VkDescriptorSet set);

	private:
		DescriptorSetLayout& m_setLayout;
		std::vector<std::pair<uint32_t, VkDescriptorImageInfo>> m_imageInfos;
		std::vector<std::pair<uint32_t, VkDescriptorBufferInfo>> m_bufferInfos;
	};



	class DescriptorSet
	{
	public:
		class Builder
		{
		public:
			Builder(DescriptorSetLayout& setLayout);
			Builder(const Builder&) = delete;
			~Builder() = default;

			auto operator=(const Builder&) noexcept -> Builder& = delete;

			auto addBuffer(uint32_t binding, const Buffer& buffer) -> Builder&;
			auto addTexture(uint32_t binding, const Texture& texture) -> Builder&;
			auto addTexture(uint32_t binding, std::shared_ptr<Texture> texture) -> Builder&;
			auto build() -> DescriptorSet;
			auto buildUnique() -> std::unique_ptr<DescriptorSet>;

		private:
			DescriptorSetLayout& m_setLayout;
			DescriptorWriter m_writer;
		};

		DescriptorSet(DescriptorSetLayout& setLayout);
		DescriptorSet(const DescriptorSet&) = delete;
		DescriptorSet(DescriptorSet&&) = default;
		~DescriptorSet() = default;

		auto operator=(const DescriptorSet&) -> DescriptorSet& = delete;
		auto operator=(DescriptorSet&&) -> DescriptorSet& = default;

		operator VkDescriptorSet() const { return m_descriptorSet; }
		auto descriptorSet() const -> VkDescriptorSet { return m_descriptorSet; }

		void bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, 
			VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

	private:
		VkDescriptorSet m_descriptorSet{ VK_NULL_HANDLE };
	};
}
