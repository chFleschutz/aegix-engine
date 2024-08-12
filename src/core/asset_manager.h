#pragma once

#include "graphics/material.h"
#include "graphics/renderer.h"
#include "graphics/model.h"

#include <memory>
#include <filesystem>
#include <type_traits>
#include <cassert>

namespace Aegix
{
	class AssetManagerGuard;

	class AssetManager
	{
	public:
		AssetManager(Graphics::Renderer& renderer) : m_renderer{ renderer } {}
		~AssetManager() = default;
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;

		static AssetManager& instance();

		template<typename T>
		std::shared_ptr<T> createMaterial()
		{
			static_assert(std::is_base_of<Graphics::BaseMaterial, T>::value, "T must derive from BaseMaterial");

			//using SystemType = typename RenderSystemRef<T>::type;
			//auto system = m_renderer.addRenderSystem<SystemType>();
			//return std::make_shared<T>(m_renderer.device(), system->descriptorSetLayout(), m_renderer.descriptorPool());
			return nullptr;
		}

		std::shared_ptr<Graphics::Model> createModel(const std::filesystem::path& modelPath);

	private:
		Graphics::Renderer& m_renderer;

		friend class AssetManagerGuard;
	};

	inline AssetManager* g_assetManager = nullptr;


	/// @brief RAII class to ensure that the AssetManager gets properly initialized and destroyed
	class AssetManagerGuard
	{
	public:
		AssetManagerGuard(Graphics::Renderer& renderer);
		~AssetManagerGuard();

	private:
		std::unique_ptr<AssetManager> m_assetManager;
	};
}