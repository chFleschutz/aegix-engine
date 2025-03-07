#pragma once

#include "graphics/systems/default_render_system.h"
#include "graphics/systems/point_light_system.h"
#include "math/math.h"
#include "scene/components.h"
#include "scene/description.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "scripting/script_base.h"
#include "utils/random.h"

/// @brief Rotates the entity around the vertical axis
/// @note Example of a custom Component
class Rotator : public Aegix::Scripting::ScriptBase
{
protected:
	void update(float deltaSeconds) override
	{
		auto& transform = component<Aegix::Transform>();
		transform.rotation *= glm::angleAxis(deltaSeconds, Aegix::Math::World::UP);
	}
};

/// @brief Scene with a teapot on a plane
/// @note Example of a custom scene
class SimpleScene : public Aegix::Scene::Description
{
public:
	/// @brief All objects in a scene are created here
	void initialize(Aegix::Scene::Scene& scene) override
	{
		using namespace Aegix;

		auto& renderer = Engine::instance().renderer();
		renderer.addRenderSystem<Aegix::Graphics::PointLightSystem>();

		// MODELS
		auto teapotMesh = Graphics::StaticMesh::create(ASSETS_DIR "Misc/teapot.obj");
		auto planeMesh = Graphics::StaticMesh::create(ASSETS_DIR "Misc/plane.obj");

		// MATERIALS
		auto textureBlack = Graphics::Texture::create({ 1, 1 }, glm::vec4{ 0.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		auto textureWhite = Graphics::Texture::create({ 1, 1 }, glm::vec4{ 1.0f }, VK_FORMAT_R8G8B8A8_UNORM);
		auto defaultNormal = Graphics::Texture::create({ 1, 1 }, glm::vec4{ 0.5f, 0.5f, 1.0f, 1.0f }, VK_FORMAT_R8G8B8A8_UNORM);

		auto paintingTexture = Graphics::Texture::create(ASSETS_DIR "Misc/painting.png", VK_FORMAT_R8G8B8A8_SRGB);
		auto paintingMat = renderer.createMaterialInstance<Aegix::Graphics::DefaultMaterial>(
			paintingTexture, defaultNormal, textureWhite, textureBlack, textureBlack);

		auto metalTexture = Graphics::Texture::create(ASSETS_DIR "Misc/brushed-metal.png", VK_FORMAT_R8G8B8A8_SRGB);
		auto metalMat = renderer.createMaterialInstance<Aegix::Graphics::DefaultMaterial>(
			metalTexture, defaultNormal, textureWhite, textureBlack, textureBlack);

		// ENTITIES
		auto floorPlane = scene.createEntity("Floor Plane");
		floorPlane.addComponent<Aegix::Mesh>(planeMesh);
		floorPlane.addComponent<Aegix::Graphics::DefaultMaterial>(paintingMat);
		floorPlane.component<Aegix::Transform>().scale = glm::vec3{ 2.0f, 2.0f, 2.0f };

		auto teapot = scene.createEntity("Teapot");
		teapot.addComponent<Aegix::Mesh>(teapotMesh);
		teapot.addComponent<Aegix::Graphics::DefaultMaterial>(metalMat);
		teapot.addComponent<Rotator>();
		teapot.component<Aegix::Transform>().scale = glm::vec3{ 2.0f, 2.0f, 2.0f };

		// LIGHTS
		constexpr int lightCount = 32;
		constexpr float lightRadius = 10.0f;

		// Ring of lights
		for (int i = 0; i < lightCount; i++)
		{
			float x = lightRadius * cos(glm::radians(360.0f / lightCount * i));
			float y = lightRadius * sin(glm::radians(360.0f / lightCount * i));
			auto light = scene.createEntity("Light " + std::to_string(i), { x, y, 3.0f });

			float r = Aegix::Random::uniformFloat(0.0f, 1.0f);
			float g = Aegix::Random::uniformFloat(0.0f, 1.0f);
			float b = Aegix::Random::uniformFloat(0.0f, 1.0f);
			light.addComponent<Aegix::PointLight>(glm::vec3{ r, g, b }, 200.0f);
		}
	}
};
