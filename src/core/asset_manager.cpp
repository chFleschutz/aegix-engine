#include "asset_manager.h"

#include <cassert>

namespace Aegix
{
	AssetManager* AssetManager::s_assetManager = nullptr;

	AssetManager::AssetManager(Graphics::Renderer& renderer)
		: m_renderer{ renderer } 
	{
		assert(s_assetManager == nullptr && "Only one instance of AssetManager is allowed");
		s_assetManager = this;
	}

	AssetManager::~AssetManager()
	{
		s_assetManager = nullptr;
	}

	AssetManager& AssetManager::instance()
	{
		assert(s_assetManager != nullptr && "AssetManager is not set");
		return *s_assetManager;
	}

	std::shared_ptr<Graphics::Model> AssetManager::createModel(const std::filesystem::path& modelPath)
	{
		return Graphics::Model::createModelFromFile(m_renderer.device(), modelPath);
	}
}
