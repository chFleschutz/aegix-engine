# VulkaniteRenderEngine

<img src="https://upload.wikimedia.org/wikipedia/commons/3/30/Vulkan.svg" width="500"/>

## Table of Contents

- [Introduction](#introduction)
- [Prerequisites](#prerequisites)
- [Getting Started](#getting-started)
- [External Libraries](#external-libraries)
- [Usage](#usage)

## Introduction <a name="introduction"></a>

Vulkanite is a simple rendering / game engine built as an exercise in preparation for my upcoming bachelor's thesis. It leverages the power of the Vulkan Graphics API for rendering, aiming to provide a foundation for advanced graphics programming. Please note that this project is a work in progress and will be continually developed alongside my thesis work.

## Prerequisites <a name="prerequisites"></a>

- Before you can run Vulkanite, you'll need to have the [Vulkan SDK](https://vulkan.lunarg.com/) installed on your system.
- CMake functions as the build system, so make sure it's installed or use an IDE with CMake support (Visual Studio is the recommended choice).

## Getting Started <a name="getting-started"></a>

Follow these steps to get started with Vulkanite Render Engine:

1. Clone the repository and its submodules:

    ```bash
    git clone --recurse-submodules https://github.com/chFleschutz/VulkaniteRenderEngine.git
    ```

2. Open the folder in Visual Studio and CMake will automatically set up the project.

4. Build and run the project to view an example scene.

## External Libraries <a name="external-libraries"></a>

This project uses several external libraries:

- [entt](https://github.com/skypjack/entt) 				(Entity Component System)
- [glfw](https://github.com/glfw/glfw) 					(Windowing)
- [glm](https://github.com/g-truc/glm) 					(Math Library)
- [stb](https://github.com/nothings/stb) 				(Image Loading)
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) 	(Loading 3D Models)
- [Vulkan](https://www.vulkan.org/) 					(Graphics API)

## Usage <a name="usage"></a>


### Creating the engine 

1. Create an instance of `vre::VulkaniteEngine` 

2. Load a scene using `loadScene<>()`
   
3. Call the `run()` function

   ```cpp
   #include "vulkanite_engine.h"
   #include "default_scene.h"

   ...

   try
   {
   	vre::VulkaniteEngine engine{};
   	engine.loadScene<DefaultScene>();
   	engine.run();
   }
   catch (const std::exception& e)
   {
   	std::cerr << e.what() << std::endl;
   }
   ```
It's recommended to use a try-catch block because exceptions are used in some cases.

### Creating a new scene:

1.  Create a subclass of `vre::Scene` and override the `initialize` method
   
    ```cpp
    #include "scene/scene.h"

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
    
7. Load the scene before calling the run function
   
   ```cpp
    vre::VulkaniteEngine engine{};
    engine.loadScene<ExampleScene>();
    engine.run();
   ```

### Custom Scripts:

1. Create a subclass of `vre::ScriptComponentBase`

   ```cpp
   #include "scene/script_component_base.h"
   
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

View [default_scene.h](src/scene/default_scene.h) for an example

