#pragma once

#include "graphics/model.h"
#include "graphics/renderer.h"
#include "graphics/systems/render_system.h"
#include "graphics/texture.h"

#include <filesystem>
#include <memory>

namespace Aegix
{
	/// @brief Used to create Assets like materials and models
	class AssetManager
	{
	public:
		~AssetManager();
		AssetManager(const AssetManager&) = delete;
		AssetManager& operator=(const AssetManager&) = delete;

		/// @brief Returns the instance of the AssetManager
		static AssetManager& instance();

		/// @brief Creates a model from a file
		/// @param modelPath Path to the model file
		/// @return Model with the data from the file
		/// @note Currently only supports .obj files
		std::shared_ptr<Graphics::Model> createModel(const std::filesystem::path& modelPath);

		/// @brief Creates a texture from a file
		/// @param texturePath Path to the texture file 
		/// @return Texture with the data from the file
		std::shared_ptr<Graphics::Texture> createTexture(const std::filesystem::path& texturePath, const Graphics::Texture::Config& config = {});

		/// @brief Adds a render system to the renderer and returns a reference to it
		/// @tparam T Type of the render system to add
		/// @note If a render system of type T already exists, it will be returned instead
		template<typename T>
		Graphics::RenderSystem& addRenderSystem()
		{
			return m_renderer.addRenderSystem<T>();
		}

		/// @brief Creates a material instance for the given material type
		/// @tparam T Type of the material for which to create an instance
		/// @return Instance of the material
		template<typename T, typename... Args>
		std::shared_ptr<typename T::Instance> createMaterialInstance(Args&&... args)
		{
			auto& system = addRenderSystem<typename T::RenderSystem>();
			return std::make_shared<typename T::Instance>(m_renderer.device(), system.descriptorSetLayout(), 
				m_renderer.globalPool(), std::forward<Args>(args)...);
		}

	private:
		AssetManager(Graphics::Renderer& renderer);

		static AssetManager* s_assetManager;

		Graphics::Renderer& m_renderer;

		friend class Engine;
	};
}