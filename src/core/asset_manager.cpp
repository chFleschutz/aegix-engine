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

	std::shared_ptr<Graphics::StaticMesh> AssetManager::createModel(const std::filesystem::path& modelPath)
	{
		return Graphics::StaticMesh::createFromFile(m_renderer.device(), ASSETS_DIR / modelPath);
	}

	std::shared_ptr<Graphics::Texture> AssetManager::createTexture(const std::filesystem::path& texturePath, VkFormat format)
	{
		return std::make_shared<Graphics::Texture>(m_renderer.device(), ASSETS_DIR / texturePath, format);
	}

	std::shared_ptr<Graphics::Texture> AssetManager::createTexture(const glm::vec4& color, uint32_t width, uint32_t height, VkFormat format)
	{
		return std::make_shared<Graphics::Texture>(m_renderer.device(), width, height, color, format);
	}

}
