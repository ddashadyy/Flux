#pragma once

#include "Window.h"

#include "Flux/Events/ApplicationEvent.h"
#include "Flux/Events/Event.h"

#include "Flux/ImGui/ImGuiLayer.h"
#include "LayerStack.h"

#include "Flux/Renderer/AssetManager.h"
#include "Flux/Renderer/FrameSync.h"
#include "Flux/Renderer/Renderer.h"
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

        void PushLayer(Layer* layer)   { m_LayerStack.PushLayer(layer); }
        void PushOverlay(Layer* layer) { m_LayerStack.PushOverlay(layer); }
        void PopLayer(Layer* layer)    { m_LayerStack.PopLayer(layer); }
        void PopOverlay(Layer* layer)  { m_LayerStack.PopOverlay(layer); }

        Window&       GetWindow()    { return *m_Window; }
        RHIDevice&    GetDevice()    { return *m_Device; }
        RHISwapchain& GetSwapchain() { return *m_Device->GetSwapchain(); }

        static Application& Get() { return *s_Instance; }

        AssetManager& GetAssetManager() { return *m_AssetManager; }
        Renderer&     GetRenderer()     { return *m_Renderer; }

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);
        void RenderFrame();


    private:
        Scope<Window>       m_Window;
        Scope<RHIDevice>    m_Device;
        Scope<FrameSync>    m_FrameSync;
        Scope<AssetManager> m_AssetManager;
        Scope<Renderer>     m_Renderer;

        uint32_t m_CurrentFrame = 0;
        uint32_t m_MaxFrames = 0;

        ImGuiLayer* m_ImGuiLayer = nullptr;
        LayerStack  m_LayerStack;
        bool        m_Running = true;

    private:
        static Application* s_Instance;
    };

    Application* CreateApplication();

}