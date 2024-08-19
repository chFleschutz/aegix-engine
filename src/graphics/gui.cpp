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

	void GUI::renderGui(VkCommandBuffer commandBuffer)
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

	void GUI::push(std::shared_ptr<Layer> layer)
	{
		assert(layer != nullptr && "Adding Layer failed: Layer is nullptr");
		m_layers.emplace_back(layer);
		layer->onAttach();
	}

	void GUI::pop(std::shared_ptr<Layer> layer)
	{
		auto it = std::find(m_layers.begin(), m_layers.end(), layer);
		if (it != m_layers.end())
		{
			m_layers.erase(it);
		}
	}
}