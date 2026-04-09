#pragma once

#include "Window.h"

#include "Flux/Events/Event.h"
#include "Flux/Events/ApplicationEvent.h"

#include "Flux/LayerStack.h"
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

        inline Window& GetWindow() { return *m_Window; }
        inline RHIDevice& GetDevice() { return *m_Device; }
        inline RHISwapchain& GetSwapchain() { return *m_Device->GetSwapchain(); }

        inline static Application& Get() { return *s_Instance; }

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);
        void RenderFrame();

    private:
        Scope<Window>     m_Window;
        Scope<RHIDevice>  m_Device;

        std::vector<Scope<RHIFence>>     m_FrameFences;
        std::vector<Scope<RHISemaphore>> m_ImageAvailable;
        std::vector<Scope<RHISemaphore>> m_RenderFinished;

        uint32_t m_CurrentFrame = 0;
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        ImGuiLayer* m_ImGuiLayer = nullptr;
        LayerStack  m_LayerStack;
        bool        m_Running = true;

    private:
        static Application* s_Instance;
    };

    Application* CreateApplication();

}