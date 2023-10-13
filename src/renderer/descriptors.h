#pragma once

#include "renderer/device.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace vre
{
    class DescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder(VulkanDevice& device) : mDevice{ device } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<DescriptorSetLayout> build() const;

        private:
            VulkanDevice& mDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> mBindings{};
        };

        DescriptorSetLayout(VulkanDevice& lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~DescriptorSetLayout();
        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

        VkDescriptorSetLayout descriptorSetLayout() const { return mDescriptorSetLayout; }

    private:
        VulkanDevice& mDevice;
        VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> mBindings;

        friend class DescriptorWriter;
    };

    class DescriptorPool
    {
    public:
        class Builder
        {
        public:
            Builder(VulkanDevice& device) : mDevice{ device } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<DescriptorPool> build() const;

        private:
            VulkanDevice& mDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        DescriptorPool(
            VulkanDevice& lveDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~DescriptorPool();
        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        bool allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        VulkanDevice& mDevice;
        VkDescriptorPool mDescriptorPool;

        friend class DescriptorWriter;
    };

    class DescriptorWriter
    {
    public:
        DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

        DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        DescriptorSetLayout& mSetLayout;
        DescriptorPool& mPool;
        std::vector<VkWriteDescriptorSet> mWrites;
    };

}  // namespace vre