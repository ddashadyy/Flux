#include "flpch.h"
#include "ImGuiLayer.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "Flux/Core/Application.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanCommandList.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Flux {

    ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}
    ImGuiLayer::~ImGuiLayer() {}

    void ImGuiLayer::OnAttach()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        Application& app = Application::Get();
        auto& vkDevice = static_cast<VulkanDevice&>(app.GetDevice());
        auto& vkSwapchain = static_cast<VulkanSwapchain&>(*app.GetDevice().GetSwapchain());
        GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

        CreateRenderPass();
        CreateFramebuffers();

        ImGui_ImplGlfw_InitForVulkan(window, true);

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.ApiVersion = VK_API_VERSION_1_2;
        initInfo.Instance = vkDevice.GetInstance();
        initInfo.PhysicalDevice = vkDevice.GetPhysicalDevice();
        initInfo.Device = vkDevice.GetHandle<VkDevice>();
        initInfo.QueueFamily = vkDevice.GetGraphicsQueueFamily();
        initInfo.Queue = vkDevice.GetGraphicsQueue();
        initInfo.DescriptorPool = vkDevice.GetDescriptorPool();
        initInfo.MinImageCount = 2;
        initInfo.ImageCount = vkSwapchain.GetImageCount();
        initInfo.PipelineInfoMain.RenderPass = m_RenderPass->GetHandle<VkRenderPass>();

        ImGui_ImplVulkan_Init(&initInfo);

        FL_CORE_INFO("ImGui Vulkan backend initialized");
    }

    void ImGuiLayer::OnDetach()
    {
        auto& vkDevice = static_cast<VulkanDevice&>(Application::Get().GetDevice());
        vkDeviceWaitIdle(vkDevice.GetHandle<VkDevice>());

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::OnImGuiRender()
    {
        static bool show = true;
        ImGui::ShowDemoWindow(&show);
    }

    void ImGuiLayer::Begin()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End(uint32_t frameIndex)
    {
        ImGuiIO& io = ImGui::GetIO();
        Application& app = Application::Get();

        io.DisplaySize = ImVec2(
            (float)app.GetWindow().GetWidth(),
            (float)app.GetWindow().GetHeight()
        );

        auto* cmdList = static_cast<VulkanCommandList*>(app.GetDevice().GetCommandList(frameIndex));
        VkCommandBuffer cmd = cmdList->GetHandle<VkCommandBuffer>();

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup);
        }
    }

    void ImGuiLayer::OnResize(uint32_t width, uint32_t height)
    {
        m_Framebuffers.clear();
        CreateFramebuffers();
    }

    void ImGuiLayer::CreateRenderPass()
    {
        Application& app = Application::Get();
        auto& vkSwapchain = static_cast<VulkanSwapchain&>(*app.GetDevice().GetSwapchain());

        RenderPassDesc rpDesc{};
        rpDesc.ColorFormats = { vkSwapchain.GetFormat() };
        rpDesc.HasDepth = false;
        rpDesc.ColorLoadOp = AttachmentLoadOp::Load;
        rpDesc.ColorStoreOp = AttachmentStoreOp::Store;
        rpDesc.ColorInitialLayout = ImageLayout::ColorAttachment;
        rpDesc.ColorFinalLayout = ImageLayout::Present;

        m_RenderPass = app.GetDevice().CreateRenderPass(rpDesc);
    }

    void ImGuiLayer::CreateFramebuffers()
    {
        Application& app = Application::Get();
        auto& vkSwapchain = static_cast<VulkanSwapchain&>(*app.GetDevice().GetSwapchain());

        uint32_t imageCount = vkSwapchain.GetImageCount();
        m_Framebuffers.resize(imageCount);

        for (uint32_t i = 0; i < imageCount; i++)
        {
            FramebufferSpec fbSpec{};
            fbSpec.RenderPass = m_RenderPass.get();
            fbSpec.ColorTargets = { vkSwapchain.GetColorTarget(i) };
            fbSpec.DepthTarget = nullptr;
            fbSpec.Width = vkSwapchain.GetExtent().width;
            fbSpec.Height = vkSwapchain.GetExtent().height;

            m_Framebuffers[i] = app.GetDevice().CreateFramebuffer(fbSpec);
        }
    }

}