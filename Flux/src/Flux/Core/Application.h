#pragma once

#include "Window.h"

#include "Flux/Events/Event.h"
#include "Flux/Events/ApplicationEvent.h"

#include "LayerStack.h"
#include "Flux/ImGui/ImGuiLayer.h"

#include "Flux/Renderer/RHIDevice.h"
#include "Flux/Renderer/RHIFactory.h"
#include "Flux/Renderer/RHISwapchain.h"

namespace Flux {

    class FLUX_API Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();
        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);
        void PopLayer(Layer* layer);
        void PopOverlay(Layer* layer);

        Window& GetWindow() { return *m_Window; }
        RHIDevice& GetDevice() { return *m_Device; }
        RHISwapchain& GetSwapchain() { return *m_Device->GetSwapchain(); }
        RHIRenderPass& GetImGuiRenderPass() const { return *m_ImGuiRenderPass; }

        static Application& Get() { return *s_Instance; }

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);
        void RenderFrame();
        
        void CreateImGuiRenderPass();
        void CreateImGuiFramebuffers();

    private:
        // Платформа
        Scope<Window>    m_Window;
        Scope<RHIDevice> m_Device;

        // Синхронизация GPU 
        std::vector<Scope<RHIFence>>     m_FrameFences;
        std::vector<Scope<RHISemaphore>> m_ImageAvailable;
        std::vector<Scope<RHISemaphore>> m_RenderFinished;

        // ImGui 
        Scope<RHIRenderPass>               m_ImGuiRenderPass;
        std::vector<Scope<RHIFramebuffer>> m_ImGuiFramebuffers;

        // Состояние фрейма
        uint32_t m_CurrentFrame = 0;
        uint32_t m_MaxFrames = 0;

        // Слои
        ImGuiLayer* m_ImGuiLayer = nullptr; // не владеет
        LayerStack  m_LayerStack;
        bool        m_Running = true;

    private:
        static Application* s_Instance;
    };

    Application* CreateApplication();

}