#pragma once

#include "Window.h"

#include "Flux/Events/Event.h"
#include "Flux/Events/ApplicationEvent.h"
#include "Flux/LayerStack.h"

#include "Flux/ImGui/ImGuiLayer.h"

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
		inline static Application& Get() { return *s_Instance; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
		bool OnAppRender(AppRenderEvent& e);

		void RenderFrame();

	private:
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer = nullptr;

		LayerStack m_LayerStack;
		bool m_Running = true;

	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}