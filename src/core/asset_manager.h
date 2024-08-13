#pragma once

#include "graphics/model.h"
#include "graphics/renderer.h"
#include "graphics/systems/render_system.h"

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

		/// @brief Creates a material instance for the given material type
		/// @tparam T Type of the material for which to create an instance
		/// @return Instance of the material
		template<typename T, typename... Args>
		std::shared_ptr<typename T::Instance> createMaterialInstance(Args&&... args)
		{
			using SystemType = typename Graphics::RenderSystemRef<T>::type;
			auto& system = m_renderer.addRenderSystem<SystemType>();
			return std::make_shared<typename T::Instance>(m_renderer.device(), system.descriptorSetLayout(), 
				m_renderer.globalPool(), std::forward<Args>(args)...);
		}

		/// @brief Creates a model from a file
		/// @param modelPath Path to the model file
		/// @return Model with the data from the file
		/// @note Currently only supports .obj files
		std::shared_ptr<Graphics::Model> createModel(const std::filesystem::path& modelPath);

	private:
		AssetManager(Graphics::Renderer& renderer);

		static AssetManager* s_assetManager;

		Graphics::Renderer& m_renderer;

		friend class Engine;
	};
}