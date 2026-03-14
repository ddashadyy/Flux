#include "flpch.h"
#include "ImGuiLayer.h"

#include <imgui.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "Flux/Application.h"
#include "Platform/Windows/WindowsWindow.h"
#include "Platform/Vulkan/VulkanContext.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Flux {

	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::StyleColorsDark();

		Application& app = Application::Get();
		auto& window = app.GetWindow();

		GLFWwindow* glfwWindow = static_cast<GLFWwindow*>(window.GetNativeWindow());

		VulkanContext& context = static_cast<VulkanContext&>(
			static_cast<WindowsWindow&>(window).GetContext()
			);

		ImGui_ImplGlfw_InitForVulkan(glfwWindow, true);

		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.ApiVersion = VK_API_VERSION_1_2;
		initInfo.Instance = context.GetInstance();
		initInfo.PhysicalDevice = context.GetPhysicalDevice();
		initInfo.Device = context.GetDevice();
		initInfo.QueueFamily = context.GetGraphicsQueueFamily();
		initInfo.Queue = context.GetGraphicsQueue();
		initInfo.DescriptorPool = context.GetDescriptorPool();
		initInfo.MinImageCount = context.GetMinImageCount();
		initInfo.ImageCount = context.GetImageCount();
		initInfo.PipelineInfoMain.RenderPass = context.GetRenderPass();

		ImGui_ImplVulkan_Init(&initInfo);

		FL_CORE_INFO("ImGui Vulkan backend initialized");
	}

	void ImGuiLayer::OnDetach()
	{
		Application& app = Application::Get();
		VulkanContext& context = static_cast<VulkanContext&>(
			static_cast<WindowsWindow&>(app.GetWindow()).GetContext()
			);

		vkDeviceWaitIdle(context.GetDevice());

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnUpdate()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		static bool show = true;
		ImGui::ShowDemoWindow(&show);

		// Своё окно
		ImGui::Begin("Flux Engine");
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::Separator();

		static float clearColor[3] = { 0.1f, 0.1f, 0.1f };
		ImGui::ColorEdit3("Clear Color", clearColor);

		if (ImGui::Button("Click me!"))
			FL_CORE_INFO("Button clicked!");

		ImGui::End();

		ImGui::Render();

		Application& app = Application::Get();
		VulkanContext& context = static_cast<VulkanContext&>(
			static_cast<WindowsWindow&>(app.GetWindow()).GetContext()
		);

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), context.GetCurrentCommandBuffer());
	}

	void ImGuiLayer::OnEvent(Event& event)
	{
	}

}