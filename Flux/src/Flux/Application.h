#pragma once

#include "Window.h"

#include "Flux/Events/Event.h"
#include "Flux/Events/ApplicationEvent.h"
#include "Flux/LayerStack.h"

#include "Window.h"

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
	private:
		bool OnWindowClose(WindowCloseEvent& e);

	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}