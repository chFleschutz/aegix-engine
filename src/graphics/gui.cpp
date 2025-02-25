#include "gui.h"

#include "graphics/renderer.h"
#include "graphics/window.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <cassert>

namespace Aegix::Graphics
{
	GUI::GUI(const Window& window, Renderer& renderer)
	{
		auto& device = renderer.device();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplGlfw_InitForVulkan(window.glfwWindow(), true);

		VkFormat colorFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = device.instance();
		initInfo.PhysicalDevice = device.physicalDevice();
		initInfo.Device = device.device();
		initInfo.QueueFamily = device.findPhysicalQueueFamilies().graphicsFamily.value();
		initInfo.Queue = device.graphicsQueue();
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.DescriptorPool = renderer.globalPool();
		initInfo.Subpass = 0;
		initInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT;
		initInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.UseDynamicRendering = true;
		initInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;
		ImGui_ImplVulkan_Init(&initInfo);

		ImGui_ImplVulkan_CreateFontsTexture();
	}

	GUI::~GUI()
	{
		for (auto& layer : m_layers)
		{
			layer->onDetach();
		}

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void GUI::update(float deltaTime)
	{
		// Cant use iterator because its possible to push/pop layers during update
		for (int i = 0; i < m_layers.size(); i++)
		{
			m_layers[i]->onUpdate(deltaTime);
		}
	}

	void GUI::render(VkCommandBuffer commandBuffer)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Cant use iterator because its possible to push/pop layers during update
		for (int i = 0; i < m_layers.size(); i++)
		{
			m_layers[i]->onGuiRender();
		}

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
	}

	void GUI::pushLayer(std::shared_ptr<Layer> layer)
	{
		assert(layer != nullptr && "Adding Layer failed: Layer is nullptr");
		m_layers.emplace_back(layer);
		layer->onAttach();
	}

	void GUI::popLayer(std::shared_ptr<Layer> layer)
	{
		auto it = std::find(m_layers.begin(), m_layers.end(), layer);
		if (it != m_layers.end())
		{
			(*it)->onDetach();
			m_layers.erase(it);
		}
	}
}