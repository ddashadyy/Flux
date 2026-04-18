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
        m_MaxFrames = m_Device->GetSwapchain()->GetImageCount();

        m_FrameSync = CreateScope<FrameSync>(*m_Device, m_MaxFrames);
        m_AssetManager = CreateScope<AssetManager>(*m_Device);
        m_Renderer = CreateScope<Renderer>(*m_Device);

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

        for (auto& layer : m_LayerStack)
            layer->OnResize(e.GetWidth(), e.GetHeight());

        return false;
    }

    void Application::RenderFrame()
    {
        auto* swapchain = m_Device->GetSwapchain();

        m_FrameSync->GetFrameFence(m_CurrentFrame).Wait();
        m_FrameSync->GetFrameFence(m_CurrentFrame).Reset();

        uint32_t imageIndex = swapchain->AcquireNextImage(&m_FrameSync->GetImageAvailable(m_CurrentFrame));
        auto* cmdList = m_Device->GetCommandList(m_CurrentFrame);

        cmdList->Begin();

        for (Layer* layer : m_LayerStack)
            layer->OnUpdate(cmdList);

        cmdList->BeginRenderPass(
            &m_ImGuiLayer->GetRenderPass(),
            &m_ImGuiLayer->GetFramebuffer(imageIndex)
        );
        cmdList->SetViewport(0, 0, (float)m_Window->GetWidth(), (float)m_Window->GetHeight());
        cmdList->SetScissor(0, 0, m_Window->GetWidth(), m_Window->GetHeight());

        m_ImGuiLayer->Begin();
        for (Layer* layer : m_LayerStack)
            layer->OnImGuiRender();
        m_ImGuiLayer->End(m_CurrentFrame);
        cmdList->EndRenderPass();

        cmdList->End();

        cmdList->Submit(
            &m_FrameSync->GetFrameFence(m_CurrentFrame),
            &m_FrameSync->GetImageAvailable(m_CurrentFrame),
            &m_FrameSync->GetRenderFinished(m_CurrentFrame)
        );

        swapchain->Present(&m_FrameSync->GetRenderFinished(m_CurrentFrame), imageIndex);

        m_CurrentFrame = (m_CurrentFrame + 1) % m_MaxFrames;
    }

}