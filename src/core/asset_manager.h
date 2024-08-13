#pragma once

#include "graphics/material.h"
#include "graphics/model.h"
#include "graphics/renderer.h"
#include "graphics/render_system.h"

#include <cassert>
#include <filesystem>
#include <memory>
#include <type_traits>

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

		/// @brief Returns the instance of the AssetManager
		static AssetManager& instance();

		/// @brief Creates a material instance 
		/// @tparam T Type of the material to create
		/// @return Instance of the material
		template<typename T>
		std::shared_ptr<T> createMaterialInstance()
		{
			static_assert(std::is_base_of<Graphics::BaseMaterial, T>::value, "T must derive from BaseMaterial");

			using SystemType = typename Graphics::RenderSystemRef<T>::type;
			auto& system = m_renderer.addRenderSystem<SystemType>();
			return std::make_shared<T>(m_renderer.device(), system.descriptorSetLayout(), m_renderer.globalPool());
		}

		/// @brief Creates a model from a file
		/// @param modelPath Path to the model file
		/// @return Model with the data from the file
		/// @note Currently only supports .obj files
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