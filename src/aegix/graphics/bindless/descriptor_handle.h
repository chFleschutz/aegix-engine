#pragma once

namespace Aegix::Graphics
{
	class DescriptorHandle
	{
		friend class DescriptorHandleCache;

	public:
		enum class Type : uint8_t
		{
			SampledTexture = 0,
			StorageTexture = 1,
			StorageBuffer = 2,
			UniformBuffer = 3,
		};

		enum class Access : uint8_t
		{
			ReadOnly = 0,
			ReadWrite = 1
		};

		static constexpr uint32_t INVALID_INDEX = std::numeric_limits<uint32_t>::max();

		DescriptorHandle() = default;
		~DescriptorHandle() = default;

		[[nodiscard]] auto index() const -> uint32_t { return m_index; }
		[[nodiscard]] auto version() const -> uint32_t { return static_cast<uint32_t>(m_version); }
		[[nodiscard]] auto type() const -> Type { return static_cast<Type>(m_type); }
		[[nodiscard]] auto access() const -> Access { return static_cast<Access>(m_access); }
		[[nodiscard]] auto isValid() const -> bool { return m_index != INVALID_INDEX; }

		void invalidate() { m_index = INVALID_INDEX; }

	private:
		DescriptorHandle(uint32_t index, Type type, Access access);

		void recycle(Type type, Access access);

		uint32_t m_index{ INVALID_INDEX };
		uint16_t m_version{ 0 };
		uint8_t m_type{ 0 };
		uint8_t m_access{ 0 };
	};
}