#pragma once

#include "graphics/static_mesh.h"
#include "graphics/systems/default_render_system.h"
#include "graphics/texture.h"
#include "scene/entity.h"

#include <gltf.h>

#include <memory>
#include <vector>

namespace Aegix::Scene
{
	class GLTFLoader
	{
	public:
		GLTFLoader(Scene& scene, const std::filesystem::path& path);

		auto rootEntity() const -> Entity { return m_rootEntity; }

	private:
		void loadScene(Scene& scene, size_t sceneIndex);
		auto toTransform(const GLTF::Node::Transform& nodeTransform) -> Transform;
		auto loadMesh(size_t meshIndex, size_t primitiveIndex) -> std::shared_ptr<Graphics::StaticMesh>;
		auto loadMaterial(size_t materialIndex) -> std::shared_ptr<Graphics::DefaultMaterialInstance>;
		auto loadTexture(size_t textureIndex, VkFormat format) -> std::shared_ptr<Graphics::Texture>;

		std::optional<GLTF::GLTF> m_gltf;
		std::vector<std::shared_ptr<Graphics::Texture>> m_textures;
		std::vector<std::shared_ptr<Graphics::DefaultMaterialInstance>> m_materials;
		std::vector<std::vector<std::shared_ptr<Graphics::StaticMesh>>> m_meshes;
		std::vector<Entity> m_entities;
		Entity m_rootEntity;

		std::shared_ptr<Graphics::Texture> m_defaultWhite;
		std::shared_ptr<Graphics::Texture> m_defaultBlack;
		std::shared_ptr<Graphics::Texture> m_defaultNormal;
		std::shared_ptr<Graphics::DefaultMaterialInstance> m_defaultMaterial;
	};
}
