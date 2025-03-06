#include "ui.h"

#include "graphics/renderer.h"
#include "graphics/window.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <cassert>

namespace Aegix::UI
{
	UI::UI(const Graphics::Window& window, Graphics::Renderer& renderer)
	{
		auto& device = renderer.device();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.IniFilename = nullptr;

#ifndef NDEBUG
		io.ConfigDebugIsDebuggerPresent = true;
#endif // !NDEBUG

		ImGui_ImplGlfw_InitForVulkan(window.glfwWindow(), true);

		VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.Instance = device.instance();
		initInfo.PhysicalDevice = device.physicalDevice();
		initInfo.Device = device.device();
		initInfo.QueueFamily = device.findPhysicalQueueFamilies().graphicsFamily.value();
		initInfo.Queue = device.graphicsQueue();
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.DescriptorPool = renderer.globalPool();
		initInfo.Subpass = 0;
		initInfo.MinImageCount = Graphics::MAX_FRAMES_IN_FLIGHT;
		initInfo.ImageCount = Graphics::MAX_FRAMES_IN_FLIGHT;
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.UseDynamicRendering = true;
		initInfo.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;
		ImGui_ImplVulkan_Init(&initInfo);

		ImGui_ImplVulkan_CreateFontsTexture();

		// Adjust Style
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 4.0f;
		style.WindowBorderSize = 0.0f;
		style.PopupRounding = 4.0f;
		style.FrameRounding = 2.0f;
		style.TouchExtraPadding = ImVec2(2.0f, 2.0f);
		style.TabBarBorderSize = 0.0f;
		style.TabRounding = 4.0f;
		style.DockingSeparatorSize = 0.0f;
	}

	UI::~UI()
	{
		for (auto& layer : m_layers)
		{
			layer->onDetach();
		}

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void UI::update(float deltaTime)
	{
		// Cant use iterator because its possible to push/pop layers during update
		for (int i = 0; i < m_layers.size(); i++)
		{
			m_layers[i]->onUpdate(deltaTime);
		}
	}

	void UI::render(VkCommandBuffer commandBuffer)
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
}