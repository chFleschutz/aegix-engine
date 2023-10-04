#pragma once

#include "device.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace vre
{
    class VreDescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder(VreDevice& device) : mVreDevice{ device } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<VreDescriptorSetLayout> build() const;

        private:
            VreDevice& mVreDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> mBindings{};
        };

        VreDescriptorSetLayout(VreDevice& lveDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~VreDescriptorSetLayout();
        VreDescriptorSetLayout(const VreDescriptorSetLayout&) = delete;
        VreDescriptorSetLayout& operator=(const VreDescriptorSetLayout&) = delete;

        VkDescriptorSetLayout descriptorSetLayout() const { return mDescriptorSetLayout; }

    private:
        VreDevice& mVreDevice;
        VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> mBindings;

        friend class VreDescriptorWriter;
    };

    class VreDescriptorPool
    {
    public:
        class Builder
        {
        public:
            Builder(VreDevice& device) : mVreDevice{ device } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<VreDescriptorPool> build() const;

        private:
            VreDevice& mVreDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        VreDescriptorPool(
            VreDevice& lveDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~VreDescriptorPool();
        VreDescriptorPool(const VreDescriptorPool&) = delete;
        VreDescriptorPool& operator=(const VreDescriptorPool&) = delete;

        bool allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        VreDevice& mVreDevice;
        VkDescriptorPool mDescriptorPool;

        friend class VreDescriptorWriter;
    };

    class VreDescriptorWriter
    {
    public:
        VreDescriptorWriter(VreDescriptorSetLayout& setLayout, VreDescriptorPool& pool);

        VreDescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        VreDescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        VreDescriptorSetLayout& mSetLayout;
        VreDescriptorPool& mPool;
        std::vector<VkWriteDescriptorSet> mWrites;
    };

}  // namespace vre