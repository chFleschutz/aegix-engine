#include "pch.h"
#include "asset_manager.h"

#include "core/globals.h"
#include "graphics/descriptors.h"
#include "graphics/pipeline.h"
#include "graphics/material/material_instance.h"
#include "graphics/material/material_template.h"
#include "engine.h"

namespace Aegix::Core
{
	void AssetManager::loadDefaultAssets()
	{
		using namespace Aegix::Graphics;

		// Default Textures

		add("default/texture_black", Texture::solidColor(glm::vec4{ 0.0f }));
		add("default/texture_white", Texture::solidColor(glm::vec4{ 1.0f }));
		add("default/texture_normal", Texture::solidColor(glm::vec4{ 0.5f, 0.5f, 1.0f, 0.0f }));

		add("default/cubemap_black", Texture::solidColorCube(glm::vec4{ 0.0f }));
		add("default/cubemap_white", Texture::solidColorCube(glm::vec4{ 1.0f }));

		// Default PBR Material
		{
			auto globalSetLayout = DescriptorSetLayout::Builder{}
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
				.build();

			auto materialSetLayout = DescriptorSetLayout::Builder{}
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();

			auto pipeline = Pipeline::GraphicsBuilder{}
				.addDescriptorSetLayout(globalSetLayout)
				.addDescriptorSetLayout(materialSetLayout)
				.addPushConstantRange(VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT, 128)
				.addShaderStages(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "pbr/default_geometry.slang.spv")
				.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
				.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
				.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
				.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
				.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
				.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
				.build();

			auto pbrMatTemplate = std::make_shared<MaterialTemplate>(std::move(pipeline), std::move(globalSetLayout), std::move(materialSetLayout));
			pbrMatTemplate->addParameter("albedo", glm::vec3{ 1.0f, 1.0f, 1.0f });
			pbrMatTemplate->addParameter("emissive", glm::vec3{ 0.0f, 0.0f, 0.0f });
			pbrMatTemplate->addParameter("metallic", 0.0f);
			pbrMatTemplate->addParameter("roughness", 1.0f);
			pbrMatTemplate->addParameter("ambientOcclusion", 1.0f);
			pbrMatTemplate->addParameter("albedoMap", get<Texture>("default/texture_white"));
			pbrMatTemplate->addParameter("normalMap", get<Texture>("default/texture_normal"));
			pbrMatTemplate->addParameter("metalRoughnessMap", get<Texture>("default/texture_white"));
			pbrMatTemplate->addParameter("ambientOcclusionMap", get<Texture>("default/texture_white"));
			pbrMatTemplate->addParameter("emissiveMap", get<Texture>("default/texture_white"));

			add("default/PBR_templatex_vertex", pbrMatTemplate);

			// Default PBR Material Instance

			auto defaultPBRMaterial = Graphics::MaterialInstance::create(pbrMatTemplate);
			defaultPBRMaterial->setParameter("albedo", glm::vec3{ 0.8f, 0.8f, 0.9f });

			add("default/PBR_instance_vertex", defaultPBRMaterial);
		}

		// Default PBR Mesh Shader Material
		{
			auto globalSetLayout = DescriptorSetLayout::Builder{}
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
				.build();

			auto materialSetLayout = DescriptorSetLayout::Builder{}
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();

			auto& bindless = Engine::renderer().bindlessDescriptorSet();

			auto meshPipeline = Pipeline::GraphicsBuilder{}
				.addDescriptorSetLayout(bindless.layout())
				.addPushConstantRange(VK_SHADER_STAGE_ALL, 128) 
				.addShaderStages(VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT, 
					SHADER_DIR "pbr/mesh_geometry_indirect.slang.spv")
				.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
				.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
				.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
				.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
				.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
				.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
				.addFlag(Pipeline::Flags::MeshShader)
				.build();

			auto pbrMeshMatTemplate = std::make_shared<MaterialTemplate>(std::move(meshPipeline), std::move(globalSetLayout), std::move(materialSetLayout));
			pbrMeshMatTemplate->addParameter("albedo", glm::vec3{ 1.0f, 1.0f, 1.0f });
			pbrMeshMatTemplate->addParameter("emissive", glm::vec3{ 0.0f, 0.0f, 0.0f });
			pbrMeshMatTemplate->addParameter("metallic", 0.0f);
			pbrMeshMatTemplate->addParameter("roughness", 1.0f);
			pbrMeshMatTemplate->addParameter("ambientOcclusion", 1.0f);
			pbrMeshMatTemplate->addParameter("albedoMap", get<Texture>("default/texture_white"));
			pbrMeshMatTemplate->addParameter("normalMap", get<Texture>("default/texture_normal"));
			pbrMeshMatTemplate->addParameter("metalRoughnessMap", get<Texture>("default/texture_white"));
			pbrMeshMatTemplate->addParameter("ambientOcclusionMap", get<Texture>("default/texture_white"));
			pbrMeshMatTemplate->addParameter("emissiveMap", get<Texture>("default/texture_white"));

			add("default/PBR_template", pbrMeshMatTemplate);

			// Default PBR Material Instance
			auto pbrMeshMat = Graphics::MaterialInstance::create(pbrMeshMatTemplate);
			pbrMeshMat->setParameter("albedo", glm::vec3{ 0.1f, 0.8f, 0.9f });
			pbrMeshMat->setParameter("metallic", 1.0f);

			add("default/PBR_instance", pbrMeshMat);
		}
	}
}