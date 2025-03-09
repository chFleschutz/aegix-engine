# Aegix Engine

A small Vulkan game engine written in modern C++. 

###

![aegix-pbs](https://github.com/user-attachments/assets/4072ddde-12f5-4e31-9715-ebb641b0be4f)

![aegix-editor](https://github.com/user-attachments/assets/0020011f-e0dc-4a6c-a0bf-33044d6ec5cf)

# Features

- **Physically Based Rendering (PBR)** - Realistic lighting and materials 
- **GLTF & OBJ Loading** - Loading of custom 3D models 
- **Immediate Mode Editor UI** - Editor tools using Dear ImGui and ImGuizmo
- **Entity Component System (ECS)** - Efficient object management using EnTT
- **Framegraph** - Multipass rendering with automatic renderpass sorting
- **Deferred Rendering** - Improved lighting performance
- **HDR Rendering and Tone Mapping** - Enables wider color range for more visible details
- **Physically Based Bloom** - Realistic glow effect around bright areas created using convolution
- **Screen Space Ambient Occlusion (SSAO)** - Enhanced depth perception

## Prerequisites <a name="prerequisites"></a>

Before building ensure you have the following installed:
- [Vulkan SDK](https://vulkan.lunarg.com/) - Download an install the latest version (at least version 1.3).
- CMake - Used for building the project. Either install it manually or use an IDE with built-in CMake support (Visual Studio is the recommended choice)

## Getting Started <a name="getting-started"></a>

Follow these steps to get started:

1. Clone the repository and its submodules:

    ```bash
    git clone --recurse-submodules https://github.com/chFleschutz/aegix-engine.git
    ```

2. Open the folder in a CMake supported IDE (e.g. Visual Studio) or generate project files for your preferred platform using CMake.

4. Build and run the project to view an example scene.

## Modules and External Libraries <a name="external-libraries"></a>

This project is structured into several independent modules:
- [aegix-assets](https://github.com/chFleschutz/aegix-assets) - Collection of sample assest
- [aegix-gltf](https://github.com/chFleschutz/aegix-gltf) - GLTF 3D model loader
- [aegix-log](https://github.com/chFleschutz/aegix-log) - Logging library

The engine relies on the following external libraries:
- [entt](https://github.com/skypjack/entt) - Entity component system (ECS)
- [glfw](https://github.com/glfw/glfw) - Window management
- [glm](https://github.com/g-truc/glm) - Math library
- [imgui](https://github.com/ocornut/imgui) - Immediate mode UI
- [imguizmo](https://github.com/CedricGuillemet/ImGuizmo) - Gizmos
- [stb](https://github.com/nothings/stb) - Image loading
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader) - OBJ 3D model loader
- [Vulkan](https://www.vulkan.org/) - Graphics API
