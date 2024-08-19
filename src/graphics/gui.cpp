#include "gui.h"

#include "graphics/swap_chain.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace Aegix::Graphics
{
	GUI::GUI(Window& window, Renderer& renderer)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplGlfw_InitForVulkan(window.glfwWindow(), true);

		auto& device = renderer.device();
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = device.instance();
		init_info.PhysicalDevice = device.physicalDevice();
		init_info.Device = device.device();
		init_info.QueueFamily = device.findPhysicalQueueFamilies().graphicsFamily;
		init_info.Queue = device.graphicsQueue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = renderer.globalPool().descriptorPool();
		init_info.RenderPass = renderer.swapChainRenderPass();
		init_info.Subpass = 0;
		init_info.MinImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		init_info.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = nullptr;
		ImGui_ImplVulkan_Init(&init_info);

		ImGui_ImplVulkan_CreateFontsTexture();
	}

	GUI::~GUI()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void GUI::beginRender()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void GUI::endRender(VkCommandBuffer commandBuffer)
	{
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
	}
}