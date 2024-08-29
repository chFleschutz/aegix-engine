#pragma once

#include "graphics/buffer.h"
#include "graphics/descriptors.h"
#include "graphics/swap_chain.h"

#include <array>

namespace Aegix::Graphics
{
	/// @brief Manages buffers for uniforms
	/// @tparam T Type of data to be stored in the buffer
	template<typename T>
	class UniformBuffer
	{
	public:
		UniformBuffer(VulkanDevice& device, const T& data = {})
			: m_data{ data }
		{
			for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
			{
				m_buffers[i] = std::make_unique<Buffer>(device, sizeof(T), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
				m_buffers[i]->map();
			}
		}

		/// @brief Returns the descriptor info for the buffer
		VkDescriptorBufferInfo descriptorInfo(int index) const { return m_buffers[index]->descriptorInfo(); }

		/// @brief Returns the data in the buffer
		const T& data() const { return m_data; }

		/// @brief Sets the data in the buffer for all frames
		/// @param data The new data
		/// @note This function should not be called once rendering has started (call setData(data, index) instead)
		void setData(const T& data)
		{
			m_data = data;
			for (auto& buffer : m_buffers)
			{
				buffer->writeToBuffer(&data);
			}
		}

		/// @brief Sets the data in the buffer for the specified frame index
		/// @param data The new data
		/// @param index The index of the currently active frame
		/// @note This function can be savely called during rendering
		void setData(int index, const T& data)
		{
			m_data = data;
			m_buffers[index]->writeToBuffer(&data);
			m_buffers[index]->flush();
		}

	private:
		T m_data;
		std::array<std::unique_ptr<Buffer>, SwapChain::MAX_FRAMES_IN_FLIGHT> m_buffers;
	};
}
