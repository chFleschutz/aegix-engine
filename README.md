# Aegix Engine

Aegix is a small game engine written in modern C++. It uses the Vulkan Graphics API for rendering, aiming to provide a foundation for advanced graphics programming. 

###

![aegix-helmet](https://github.com/user-attachments/assets/f77b25d8-bd15-49df-bc4f-47285141c251)

![aegix-editor](https://github.com/user-attachments/assets/86b8bafd-462d-4a64-95da-b348e024ae0a)


## Prerequisites <a name="prerequisites"></a>

- Before you can run Aegix, you'll need to have the [Vulkan SDK](https://vulkan.lunarg.com/) installed on your system.
- CMake functions as the build system, so make sure it's installed or use an IDE with CMake support (Visual Studio is the recommended choice).

## Getting Started <a name="getting-started"></a>

Follow these steps to get started with the Aegix Engine:

1. Clone the repository and its submodules:

    ```bash
    git clone --recurse-submodules https://github.com/chFleschutz/aegix-engine.git
    ```

2. Open the folder in Visual Studio and CMake will automatically set up the project.

4. Build and run the project to view an example scene.

## External Libraries <a name="external-libraries"></a>

This project uses several external libraries:

- [entt](https://github.com/skypjack/entt): Entity Component System
- [glfw](https://github.com/glfw/glfw): Windowing
- [glm](https://github.com/g-truc/glm): Math Library
- [imgui](https://github.com/ocornut/imgui): UI
- [stb](https://github.com/nothings/stb): Image Loading
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader): Loading 3D Models
- [Vulkan](https://www.vulkan.org/): Graphics API
