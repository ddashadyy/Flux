#include "flpch.h"
#include "Application.h"
#include "Flux/Log.h"
#include "Flux/Core.h"
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

        m_FrameFences.resize(MAX_FRAMES_IN_FLIGHT);
        m_ImageAvailable.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinished.resize(MAX_FRAMES_IN_FLIGHT);

        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            m_FrameFences[i] = m_Device->CreateFence(true);
            m_ImageAvailable[i] = m_Device->CreateSemaphore();
            m_RenderFinished[i] = m_Device->CreateSemaphore();
        }

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
        while (m_Running) {
            RenderFrame();
            m_Window->OnUpdate();
        }
    }

    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(FL_BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(FL_BIND_EVENT_FN(Application::OnWindowResize));

        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
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
        m_Device->GetSwapchain()->Resize(e.GetWidth(), e.GetHeight());
        return false;
    }

    void Application::RenderFrame()
    {
        auto* swapchain = m_Device->GetSwapchain();
        auto* cmdList = m_Device->GetCommandList();

        m_FrameFences[m_CurrentFrame]->Wait();
        m_FrameFences[m_CurrentFrame]->Reset();

        swapchain->AcquireNextImage(m_ImageAvailable[m_CurrentFrame].get());

        cmdList->Begin();

        for (Layer* layer : m_LayerStack)
            layer->OnUpdate();

        m_ImGuiLayer->Begin();
        for (Layer* layer : m_LayerStack)
            layer->OnImGuiRender();
        m_ImGuiLayer->End();

        cmdList->End();
        cmdList->Submit(
            m_FrameFences[m_CurrentFrame].get(),
            m_ImageAvailable[m_CurrentFrame].get(),
            m_RenderFinished[m_CurrentFrame].get()
        );

        swapchain->Present(m_RenderFinished[m_CurrentFrame].get());

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

}