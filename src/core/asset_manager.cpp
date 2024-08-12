#include "asset_manager.h"

namespace Aegix
{
	AssetManager& AssetManager::instance()
	{
		assert(g_assetManager != nullptr);
		return *g_assetManager;
	}

	std::shared_ptr<Graphics::Model> AssetManager::createModel(const std::filesystem::path& modelPath)
	{
		return Graphics::Model::createModelFromFile(m_renderer.device(), modelPath);
	}



	AssetManagerGuard::AssetManagerGuard(Graphics::Renderer& renderer)
	{
		m_assetManager = std::make_unique<AssetManager>(renderer);
		g_assetManager = m_assetManager.get();
	}

	AssetManagerGuard::~AssetManagerGuard()
	{
		g_assetManager = nullptr;
		m_assetManager.reset();
	}
}
