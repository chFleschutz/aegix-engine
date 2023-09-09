# VulkaniteRenderEngine

<img src="https://upload.wikimedia.org/wikipedia/commons/3/30/Vulkan.svg" width="500"/>

## Introduction

Vulkanite Render Engine is a simple rendering engine built as an exercise in preparation for my upcoming bachelor's thesis. It leverages the power of the Vulkan Graphics API for rendering, aiming to provide a foundation for advanced graphics programming. Please note that this project is a work in progress and will be continually developed alongside my thesis work.

## Prerequisites

Before you can run Vulkanite Render Engine, you'll need to have the following prerequisites in place:

- [Vulkan SDK](https://vulkan.lunarg.com/) installed on your system.

- Download [GLFW](https://www.glfw.org/download.html) (version 3.3.8, but the latest should also work).

## Getting Started

Follow these steps to get started with Vulkanite Render Engine:

1. Clone the repository:

    ```bash
    git clone https://github.com/chFleschutz/VulkaniteRenderEngine.git
    ```

2. Open the Visual Studio solution.

3. Update the GLFW location in project properties (ensure you select all configurations):

    - Project Properties -> C/C++ -> General -> Additional Include Directories: Update the GLFW include path.
    
    - Project Properties -> Linker -> General -> Additional Library Directories: Update the GLFW lib path.

4. Build and run the project.

## External Libraries

This project uses several external libraries to enhance its functionality:

- [GLFW](https://github.com/glfw/glfw)
- [GLM](https://github.com/g-truc/glm)
- [stb](https://github.com/nothings/stb)
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
- [Vulkan](https://www.vulkan.org/)

Feel free to explore these libraries to understand their contributions to Vulkanite Render Engine.

