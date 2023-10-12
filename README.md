# VulkaniteRenderEngine

<img src="https://upload.wikimedia.org/wikipedia/commons/3/30/Vulkan.svg" width="500"/>

## Table of Contents

- [Introduction](#introduction)
- [Prerequisites](#prerequisites)
- [Getting Started](#getting-started)
- [External Libraries](#external-libraries)
- [Usage](#usage)

## Introduction <a name="introduction"></a>

Vulkanite Render Engine is a simple rendering engine built as an exercise in preparation for my upcoming bachelor's thesis. It leverages the power of the Vulkan Graphics API for rendering, aiming to provide a foundation for advanced graphics programming. Please note that this project is a work in progress and will be continually developed alongside my thesis work.

## Prerequisites <a name="prerequisites"></a>

Before you can run Vulkanite, you'll need to have the [Vulkan SDK](https://vulkan.lunarg.com/) installed on your system.

## Getting Started <a name="getting-started"></a>

Follow these steps to get started with Vulkanite Render Engine:

1. Clone the repository and its submodules:

    ```bash
    git clone --recurse-submodules https://github.com/chFleschutz/VulkaniteRenderEngine.git
    ```

2. Open the Visual Studio solution.

4. Build and run the project to view an example scene.

## External Libraries <a name="external-libraries"></a>

This project uses several external libraries to enhance its functionality:

- [GLFW](https://github.com/glfw/glfw)
- [GLM](https://github.com/g-truc/glm)
- [stb](https://github.com/nothings/stb)
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
- [Vulkan](https://www.vulkan.org/)

## Usage <a name="usage"></a>

View [default_scene.h](src/default_scene.h) for a complete example

### Creating a new scene:

1.  Create a subclass of `vre::Scene` and override the `initialize` method
   
    ```cpp
    #include "scene.h"

    class ExampleScene : public vre::Scene
    {
    public:
    	using vre::Scene::Scene;
    
    	void initialize() override;
    };
    ```
    
5.  Fill the scene with entities and lights
   
    ```cpp
    void ExampleScene::initialize()
    {
    	auto teapotModel = loadModel("models/teapot.obj");
    	auto teapot = createEntity("Teapot");
    	teapot.addComponent<vre::MeshComponent>(teapotModel);

    	auto light1 = createEntity("Light 1", { -1.0f, -1.0f, -1.0f });
    	light1.addComponent<vre::PointLightComponent>();
    }
    ```
    
7. Load the scene after creating the engine
   
   ```cpp
    vre::VulkaniteEngine engine{};
    engine.loadScene<ExampleScene>();
    engine.run();
   ```

### Custom Scripts:

1. Create a subclass of `vre::ScriptComponentBase`

   ```cpp
   class ExampleScript : public vre::ScriptComponentBase
   {
    
   };
   ```

3. Add functionality by overriding `begin`, `update` or `end`

   ```cpp
	void begin() override;                      // Called once at the beginning
	void update(float deltaSeconds) override;   // Called once per frame 
	void end() override;                        // Called once after the last frame
   ```
   
4. Add the script to an entity in your `Scene::initialize` method

   ```cpp
    myEntity.addScript<ExampleScript>();
   ```

