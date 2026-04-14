#include "flpch.h"
#include "Application.h"
#include "Log.h"
#include "Base.h"
#include "Input.h"
#include <GLFW/glfw3.h>

namespace Flux {

    Application* Application::s_Instance = nullptr;

    Application::Application()
    {
        FL_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        m_Window = Scope<Window>(Window::Create());
        m_Window->SetEventCallback(FL_BIND_EVENT_FN(Application::OnEvent));

        void* nativeWindow = m_Window->GetNativeWindow();
        uint32_t width = m_Window->GetWidth();
        uint32_t height = m_Window->GetHeight();

        m_Device = RHIFactory::CreateDevice(RendererBackend::Auto, nativeWindow, width, height);

        auto* swapchain = m_Device->GetSwapchain();
        m_MaxFrames = swapchain->GetImageCount();

        m_FrameFences.resize(m_MaxFrames);
        m_ImageAvailable.resize(m_MaxFrames);
        m_RenderFinished.resize(m_MaxFrames);

        for (uint32_t i = 0; i < m_MaxFrames; i++) 
        {
            m_FrameFences[i] = m_Device->CreateFence(true);
            m_ImageAvailable[i] = m_Device->CreateSemaphore();
            m_RenderFinished[i] = m_Device->CreateSemaphore();
        }

        CreateImGuiRenderPass();      
        CreateImGuiFramebuffers();    

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);
    }

    Application::~Application()
    {
        PopOverlay(m_ImGuiLayer);
        delete m_ImGuiLayer;
    }

    void Application::Run()
    {
        while (m_Running) 
        {
            RenderFrame();
            m_Window->OnUpdate();
        }
    }

    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(FL_BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(FL_BIND_EVENT_FN(Application::OnWindowResize));

        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) 
        {
            (*--it)->OnEvent(e);
            if (e.Handled) break;
        }
    }

    void Application::PushLayer(Layer* layer) { m_LayerStack.PushLayer(layer); }
    void Application::PushOverlay(Layer* layer) { m_LayerStack.PushOverlay(layer); }
    void Application::PopLayer(Layer* layer) { m_LayerStack.PopLayer(layer); }
    void Application::PopOverlay(Layer* layer) { m_LayerStack.PopOverlay(layer); }

    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e)
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
            return false;

        m_Device->GetSwapchain()->Resize(e.GetWidth(), e.GetHeight());
        CreateImGuiFramebuffers(); 

        for (auto& layer : m_LayerStack)
            layer->OnResize(e.GetWidth(), e.GetHeight());

        return false;
    }

    void Application::RenderFrame()
    {
        auto* swapchain = m_Device->GetSwapchain();

        m_FrameFences[m_CurrentFrame]->Wait();
        m_FrameFences[m_CurrentFrame]->Reset();

        uint32_t imageIndex = swapchain->AcquireNextImage(m_ImageAvailable[m_CurrentFrame].get());
        auto* cmdList = m_Device->GetCommandList(m_CurrentFrame);

        cmdList->Begin();

        for (Layer* layer : m_LayerStack)
            layer->OnUpdate(cmdList);

        cmdList->BeginRenderPass(m_ImGuiRenderPass.get(), m_ImGuiFramebuffers[imageIndex].get());
        cmdList->SetViewport(0, 0, (float)m_Window->GetWidth(), (float)m_Window->GetHeight());
        cmdList->SetScissor(0, 0, m_Window->GetWidth(), m_Window->GetHeight());

        m_ImGuiLayer->Begin();
        for (Layer* layer : m_LayerStack)
            layer->OnImGuiRender();
        m_ImGuiLayer->End(m_CurrentFrame);
        cmdList->EndRenderPass();

        cmdList->End();

        cmdList->Submit(
            m_FrameFences[m_CurrentFrame].get(),
            m_ImageAvailable[m_CurrentFrame].get(),
            m_RenderFinished[m_CurrentFrame].get()
        );

        swapchain->Present(m_RenderFinished[m_CurrentFrame].get(), imageIndex);

        m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFrames;
    }

    void Application::CreateImGuiRenderPass()
    {
        auto* swapchain = m_Device->GetSwapchain();

        RenderPassDesc rpDesc{};
        rpDesc.ColorFormats = { swapchain->GetFormat() };
        rpDesc.HasDepth = false;
        rpDesc.ColorLoadOp = AttachmentLoadOp::Load;        
        rpDesc.ColorStoreOp = AttachmentStoreOp::Store;
        rpDesc.ColorInitialLayout = ImageLayout::ColorAttachment;
        rpDesc.ColorFinalLayout = ImageLayout::Present;     
        m_ImGuiRenderPass = m_Device->CreateRenderPass(rpDesc);
    }

    void Application::CreateImGuiFramebuffers()
    {
        auto* swapchain = m_Device->GetSwapchain();
        uint32_t imageCount = swapchain->GetImageCount();
        m_ImGuiFramebuffers.resize(imageCount);

        for (uint32_t i = 0; i < imageCount; i++)
        {
            FramebufferSpec fbSpec{};
            fbSpec.RenderPass = m_ImGuiRenderPass.get();
            fbSpec.ColorTargets = { swapchain->GetColorTarget(i) };
            fbSpec.DepthTarget = nullptr;
            fbSpec.Width = m_Window->GetWidth();
            fbSpec.Height = m_Window->GetHeight();
            m_ImGuiFramebuffers[i] = m_Device->CreateFramebuffer(fbSpec);
        }
    }

}