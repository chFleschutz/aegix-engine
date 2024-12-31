#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <memory>

namespace Aegix::Graphics
{
	using FrameGraphResourceID = uint32_t;

	/// @brief Wrapper for a resource in the frame graph
	/// @details This class uses template type erasure to store different types of resources
	class FrameGraphResource
	{
	public:
		template <typename T>
		FrameGraphResource(FrameGraphResourceID id, const typename T::Desc& desc, T&& obj)
			: m_id{ id }, m_resource{ std::make_unique<Resource<T>>(desc, std::forward<T>(obj)) }
		{
		}

	private:
		struct ResourceBase
		{
			virtual ~ResourceBase() = default;
		};

		template <typename T>
		struct Resource : public ResourceBase
		{
			Resource(const typename T::Desc& desc, T&& obj)
				: desc{ desc }, resource{ std::move(obj) }
			{
			}

			typename T::Desc desc;
			T resource;
		};

		FrameGraphResourceID m_id;
		std::unique_ptr<ResourceBase> m_resource;
	};
}