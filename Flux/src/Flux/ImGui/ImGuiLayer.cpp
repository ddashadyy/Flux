#include "flpch.h"
#include "ImGuiLayer.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "Flux/Core/Application.h"
#include "Flux/Editor/EditorTheme.h"

#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanSwapchain.h"
#include "Platform/Vulkan/VulkanCommandList.h"

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

        EditorTheme::Apply();

        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        Application& app = Application::Get();
        auto& vkDevice = static_cast<VulkanDevice&>(app.GetDevice());
        auto* swapchain = app.GetDevice().GetSwapchain();
        GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

        ImGui_ImplGlfw_InitForVulkan(window, true);

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.ApiVersion = VK_API_VERSION_1_2;
        initInfo.Instance = vkDevice.GetInstance();
        initInfo.PhysicalDevice = vkDevice.GetPhysicalDevice();
        initInfo.Device = static_cast<VkDevice>(vkDevice.GetHandle());
        initInfo.QueueFamily = vkDevice.GetGraphicsFamily();
        initInfo.Queue = vkDevice.GetGraphicsQueue();
        initInfo.DescriptorPool = vkDevice.GetDescriptorPool();
        initInfo.MinImageCount = 2;
        initInfo.ImageCount = swapchain->GetImageCount();

        RenderPassDesc desc{};
        desc.ColorFormats = { swapchain->GetFormat() };
        desc.HasDepth = false;
        desc.Samples = SampleCount::x1;
        desc.ColorLoadOp = AttachmentLoadOp::Clear;
        desc.ColorStoreOp = AttachmentStoreOp::Store;
        desc.ColorInitialLayout = ImageLayout::Undefined;
        desc.ColorFinalLayout = ImageLayout::Present; 
        m_PresentRenderPass = app.GetDevice().CreateRenderPass(desc);

        initInfo.PipelineInfoMain.RenderPass = static_cast<VkRenderPass>(m_PresentRenderPass->GetHandle());
        ImGui_ImplVulkan_Init(&initInfo);

        CreateFramebuffers(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());

        FL_CORE_INFO("ImGui Vulkan backend initialized with custom Present RenderPass");
    }

    void ImGuiLayer::OnDetach()
    {
        Application::Get().GetDevice().WaitIdle();
        DestroyFramebuffers();
        m_PresentRenderPass.reset();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::OnImGuiRender()
    {
    }

    void ImGuiLayer::Begin()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End(uint32_t frameIndex)
    {
        ImGui::Render();

        Application& app = Application::Get();
        auto* cmdList = static_cast<VulkanCommandList*>(app.GetDevice().GetCommandList(frameIndex));
        VkCommandBuffer cmd = static_cast<VkCommandBuffer>(cmdList->GetHandle());

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    void ImGuiLayer::CreateFramebuffers(uint32_t width, uint32_t height)
    {
        DestroyFramebuffers();

        auto* swapchain = Application::Get().GetDevice().GetSwapchain();
        uint32_t count = swapchain->GetImageCount();
        m_Framebuffers.resize(count);

        for (uint32_t i = 0; i < count; i++)
        {
            FramebufferSpec spec{};
            spec.RenderPass = m_PresentRenderPass.get();
            spec.Width = width;
            spec.Height = height;
            spec.ColorTargets = { swapchain->GetColorTarget(i) };
            m_Framebuffers[i] = Application::Get().GetDevice().CreateFramebuffer(spec);
        }
    }
    void ImGuiLayer::DestroyFramebuffers()
    {
        m_Framebuffers.clear();
    }
}