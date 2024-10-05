#include "ui_pass.h"

#include "core/engine.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace Aegix::Graphics
{
	UiPass::UiPass(VulkanDevice& device, const Window& window, VkDescriptorPool globalPool, VkRenderPass renderpass)
		: RenderPass(device)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplGlfw_InitForVulkan(window.glfwWindow(), true);

		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = device.instance();
		init_info.PhysicalDevice = device.physicalDevice();
		init_info.Device = device.device();
		init_info.QueueFamily = device.findPhysicalQueueFamilies().graphicsFamily.value();
		init_info.Queue = device.graphicsQueue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = globalPool;
		init_info.RenderPass = renderpass;
		init_info.Subpass = 0;
		init_info.MinImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		init_info.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = nullptr;
		ImGui_ImplVulkan_Init(&init_info);

		ImGui_ImplVulkan_CreateFontsTexture();
	}

	UiPass::~UiPass()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void UiPass::render(FrameInfo& frameInfo)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		Engine::instance().gui().renderGui();

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
	}
}