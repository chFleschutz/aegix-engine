#pragma once

#include "core/asset.h"

namespace Aegix::Core
{
	class AssetManager
	{
	public:
		AssetManager() = default;
		~AssetManager() = default;

		template<IsAsset T>
		//requires IsLoadable<T> // TODO: Add loadable concept
		[[nodiscard]] auto get(const std::filesystem::path& path) -> std::shared_ptr<T>
		{
			auto it = m_assets.find(path);
			if (it != m_assets.end())
				return std::static_pointer_cast<T>(it->second);

			// TODO: Implement actual loading logic
			//std::shared_ptr<T> newAsset = T::load(path);
			//AGX_ASSERT(newAsset, "Failed to load asset");
			//m_assets[path] = newAsset;
			//return newAsset;

			AGX_ASSERT_X(false, "Asset loading not implemented yet");
			return nullptr;
		}

		template<IsAsset T>
		void add(const std::filesystem::path& path, const std::shared_ptr<T>& asset)
		{
			m_assets[path] = asset;
		}

		void garbageCollect()
		{
			for (auto it = m_assets.begin(); it != m_assets.end(); )
			{
				if (it->second.use_count() == 1)
					it = m_assets.erase(it);
				else
					++it;
			}
		}
		
		void loadDefaultAssets();

	private:
		// TODO: Use a weak_ptr for auto release of assets (needs asset file loading first to load on demand)
		std::unordered_map<std::filesystem::path, std::shared_ptr<Asset>> m_assets;
	};
}