# VulkaniteRenderEngine

Vulkanite is a simple Render Engine created as an exercise.   
Like the name implies it uses the Vulkan API for rendering. 

## External Libaries:
- [glfw](https://github.com/glfw/glfw)
- [glm](https://github.com/g-truc/glm)
- [stb](https://github.com/nothings/stb)
- [tiny_obj_loader](https://github.com/tinyobjloader/tinyobjloader)
- [Vulkan](https://www.vulkan.org/)

## Project setup:
- Install the Vulkan SDK from https://vulkan.lunarg.com/ 
- Download glfw from https://www.glfw.org/download.html (version 3.3.8 but latest should also work)
- Clone the repository and open the vs solution
- Update the glfw location in project properties (make sure to select all configurations)
  - project properties -> C/C++ -> General -> Additional Include Directories -> Update the glfw include path
  - project properties -> Linker -> General -> Additional Library Directories -> Update the glfw lib path
- Build and run 
