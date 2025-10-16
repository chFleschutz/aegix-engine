#include "asset_manager.h"

#include "core/globals.h"
#include "graphics/descriptors.h"
#include "graphics/pipeline.h"
#include "graphics/resources/material_instance.h"
#include "graphics/resources/material_template.h"

namespace Aegix::Core
{
	void AssetManager::loadDefaultAssets()
	{
		using namespace Aegix::Graphics;

		// Default Textures

		add("default/texture_black", Texture::create(glm::vec4{ 0.0f }, VK_FORMAT_R8G8B8A8_UNORM));
		add("default/texture_white", Texture::create(glm::vec4{ 1.0f }, VK_FORMAT_R8G8B8A8_UNORM));
		add("default/texture_normal", Texture::create(glm::vec4{ 0.5f, 0.5f, 1.0f, 0.0f }, VK_FORMAT_R8G8B8A8_UNORM));


		// Default PBR Material
		{
			auto globalSetLayout = DescriptorSetLayout::Builder{}
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
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
				.addPushConstantRange(VK_SHADER_STAGE_ALL_GRAPHICS, 128)
				.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "pbr/default_geometry.vert.spv")
				.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "pbr/default_geometry.frag.spv")
				.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
				.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
				.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
				.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
				.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
				.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
				.build();

			auto pbrMatTemplate = std::make_shared<MaterialTemplate>(std::move(pipeline), std::move(globalSetLayout), std::move(materialSetLayout));
			pbrMatTemplate->addParameter("albedo", MaterialParamType::Vec3, glm::vec3{ 1.0f, 1.0f, 1.0f });
			pbrMatTemplate->addParameter("emissive", MaterialParamType::Vec3, glm::vec3{ 0.0f, 0.0f, 0.0f });
			pbrMatTemplate->addParameter("metallic", MaterialParamType::Float, 0.0f);
			pbrMatTemplate->addParameter("roughness", MaterialParamType::Float, 1.0f);
			pbrMatTemplate->addParameter("ambientOcclusion", MaterialParamType::Float, 1.0f);
			pbrMatTemplate->addParameter("albedoMap", MaterialParamType::Texture2D, get<Texture>("default/texture_white"));
			pbrMatTemplate->addParameter("normalMap", MaterialParamType::Texture2D, get<Texture>("default/texture_normal"));
			pbrMatTemplate->addParameter("metalRoughnessMap", MaterialParamType::Texture2D, get<Texture>("default/texture_white"));
			pbrMatTemplate->addParameter("ambientOcclusionMap", MaterialParamType::Texture2D, get<Texture>("default/texture_white"));
			pbrMatTemplate->addParameter("emissiveMap", MaterialParamType::Texture2D, get<Texture>("default/texture_white"));

			add("default/PBR_template", pbrMatTemplate);

			// Default PBR Material Instance

			auto defaultPBRMaterial = Graphics::MaterialInstance::create(pbrMatTemplate);
			defaultPBRMaterial->setParameter("albedo", glm::vec3{ 0.8f, 0.8f, 0.9f });

			add("default/PBR_instance", defaultPBRMaterial);
		}

		// Default PBR Mesh Shader Material
		{
			auto globalSetLayout = DescriptorSetLayout::Builder{}
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();

			auto materialSetLayout = DescriptorSetLayout::Builder{}
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();

			auto meshPipeline = Pipeline::GraphicsBuilder{}
				.addDescriptorSetLayout(globalSetLayout)
				.addDescriptorSetLayout(materialSetLayout)
				.addPushConstantRange(VK_SHADER_STAGE_ALL_GRAPHICS, 128)
				.addShaderStage(VK_SHADER_STAGE_TASK_BIT_EXT, SHADER_DIR "pbr/mesh_geometry.task.spv")
				.addShaderStage(VK_SHADER_STAGE_MESH_BIT_EXT, SHADER_DIR "pbr/mesh_geometry.mesh.spv")
				.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "pbr/default_geometry.frag.spv")
				.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
				.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
				.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
				.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
				.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
				.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
				.disableVertexInput()
				.build();

			auto pbrMeshMatTemplate = std::make_shared<MaterialTemplate>(std::move(meshPipeline), std::move(globalSetLayout), std::move(materialSetLayout));
			pbrMeshMatTemplate->addParameter("albedo", MaterialParamType::Vec3, glm::vec3{ 1.0f, 1.0f, 1.0f });
			pbrMeshMatTemplate->addParameter("emissive", MaterialParamType::Vec3, glm::vec3{ 0.0f, 0.0f, 0.0f });
			pbrMeshMatTemplate->addParameter("metallic", MaterialParamType::Float, 0.0f);
			pbrMeshMatTemplate->addParameter("roughness", MaterialParamType::Float, 1.0f);
			pbrMeshMatTemplate->addParameter("ambientOcclusion", MaterialParamType::Float, 1.0f);
			pbrMeshMatTemplate->addParameter("albedoMap", MaterialParamType::Texture2D, get<Texture>("default/texture_white"));
			pbrMeshMatTemplate->addParameter("normalMap", MaterialParamType::Texture2D, get<Texture>("default/texture_normal"));
			pbrMeshMatTemplate->addParameter("metalRoughnessMap", MaterialParamType::Texture2D, get<Texture>("default/texture_white"));
			pbrMeshMatTemplate->addParameter("ambientOcclusionMap", MaterialParamType::Texture2D, get<Texture>("default/texture_white"));
			pbrMeshMatTemplate->addParameter("emissiveMap", MaterialParamType::Texture2D, get<Texture>("default/texture_white"));

			add("default/PBR_mesh_template", pbrMeshMatTemplate);

			// Default PBR Material Instance
			auto pbrMeshMat = Graphics::MaterialInstance::create(pbrMeshMatTemplate);
			pbrMeshMat->setParameter("albedo", glm::vec3{ 0.1f, 0.8f, 0.9f });
			pbrMeshMat->setParameter("metallic", 1.0f);

			add("default/PBR_mesh_instance", pbrMeshMat);
		}
	}
}