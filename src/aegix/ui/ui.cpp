#include "pch.h"

#include "ui.h"

#include "core/profiler.h"
#include "graphics/graphics.h"
#include "graphics/vulkan/volk_include.h"
#include "graphics/vulkan_context.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <ImGuizmo.h>

namespace Aegix::UI
{
	UI::UI(Graphics::Graphics& graphics, Core::LayerStack& layerStack)
		: m_layerStack{ layerStack }
	{
		auto& renderer = graphics.renderer();
		auto& device = Graphics::VulkanContext::device();
		auto& window = renderer.window();

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

		VkFormat colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
		ImGui_ImplVulkan_InitInfo initInfo{
			.ApiVersion = Graphics::VulkanDevice::API_VERSION,
			.Instance = device.instance(),
			.PhysicalDevice = device.physicalDevice(),
			.Device = device.device(),
			.QueueFamily = device.findPhysicalQueueFamilies().graphicsFamily.value(),
			.Queue = device.graphicsQueue(),
			.DescriptorPool = Graphics::VulkanContext::descriptorPool(),
			.MinImageCount = Graphics::MAX_FRAMES_IN_FLIGHT,
			.ImageCount = Graphics::MAX_FRAMES_IN_FLIGHT,
			.PipelineInfoMain = {
				.RenderPass = VK_NULL_HANDLE,
				.Subpass = 0,
				.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
				.PipelineRenderingCreateInfo = {
					.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
					.colorAttachmentCount = 1,
					.pColorAttachmentFormats = &colorFormat,
				}
			},
			.UseDynamicRendering = true,
		};

		ImGui_ImplVulkan_Init(&initInfo);

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
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void UI::render(VkCommandBuffer commandBuffer)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		// Create full screen window to receive mouse events everywhere
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Main Window", nullptr, windowFlags);
		ImGui::PopStyleVar(3);

		for (auto& layer : m_layerStack)
		{
			layer->onUIRender();
		}

		ImGui::End();

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
	}
}