#pragma once

namespace Aegix::Core
{
	class Asset
	{
	public:
		virtual ~Asset() = default;

		[[nodiscard]] auto path() const -> const std::filesystem::path& { return m_path; }

	protected:
		std::filesystem::path m_path;
	};

	template<typename T>
	concept IsAsset = std::derived_from<T, Asset>;

	template<typename T>
	concept IsLoadable = requires(const std::filesystem::path& path)
	{
		{ T::load(path) } -> std::same_as<std::shared_ptr<T>>;
	};
}