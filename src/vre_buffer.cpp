#include "vre_buffer.h"

#include <cassert>
#include <cstring>

namespace vre
{
    VkDeviceSize VreBuffer::getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
    {
        if (minOffsetAlignment > 0)
        {
            return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
        }
        return instanceSize;
    }

    VreBuffer::VreBuffer(
        VreDevice& device,
        VkDeviceSize instanceSize,
        uint32_t instanceCount,
        VkBufferUsageFlags usageFlags,
        VkMemoryPropertyFlags memoryPropertyFlags,
        VkDeviceSize minOffsetAlignment)
        : 
        mVreDevice{ device },
        mInstanceSize{ instanceSize },
        mInstanceCount{ instanceCount },
        mUsageFlags{ usageFlags },
        mMemoryPropertyFlags{ memoryPropertyFlags }
    {
        mAlignmentSize = getAlignment(instanceSize, minOffsetAlignment);
        mBufferSize = mAlignmentSize * instanceCount;
        device.createBuffer(mBufferSize, usageFlags, memoryPropertyFlags, mBuffer, mMemory);
    }

    VreBuffer::~VreBuffer()
    {
        unmap();
        vkDestroyBuffer(mVreDevice.device(), mBuffer, nullptr);
        vkFreeMemory(mVreDevice.device(), mMemory, nullptr);
    }

    VkResult VreBuffer::map(VkDeviceSize size, VkDeviceSize offset)
    {
        assert(mBuffer && mMemory && "Called map on buffer before create");
        return vkMapMemory(mVreDevice.device(), mMemory, offset, size, 0, &mMapped);
    }

    void VreBuffer::unmap()
    {
        if (mMapped)
        {
            vkUnmapMemory(mVreDevice.device(), mMemory);
            mMapped = nullptr;
        }
    }

    void VreBuffer::writeToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset)
    {
        assert(mMapped && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE)
        {
            memcpy(mMapped, data, mBufferSize);
        }
        else
        {
            char* memOffset = (char*)mMapped;
            memOffset += offset;
            memcpy(memOffset, data, size);
        }
    }

    VkResult VreBuffer::flush(VkDeviceSize size, VkDeviceSize offset)
    {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = mMemory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkFlushMappedMemoryRanges(mVreDevice.device(), 1, &mappedRange);
    }

    VkResult VreBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
    {
        VkMappedMemoryRange mappedRange = {};
        mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        mappedRange.memory = mMemory;
        mappedRange.offset = offset;
        mappedRange.size = size;
        return vkInvalidateMappedMemoryRanges(mVreDevice.device(), 1, &mappedRange);
    }

    VkDescriptorBufferInfo VreBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset)
    {
        return VkDescriptorBufferInfo{
            mBuffer,
            offset,
            size,
        };
    }

    void VreBuffer::writeToIndex(void* data, int index)
    {
        writeToBuffer(data, mInstanceSize, index * mAlignmentSize);
    }

    VkResult VreBuffer::flushIndex(int index) 
    { 
        return flush(mAlignmentSize, index * mAlignmentSize); 
    }

    VkDescriptorBufferInfo VreBuffer::descriptorInfoForIndex(int index)
    {
        return descriptorInfo(mAlignmentSize, index * mAlignmentSize);
    }

    VkResult VreBuffer::invalidateIndex(int index)
    {
        return invalidate(mAlignmentSize, index * mAlignmentSize);
    }

}  // namespace lve