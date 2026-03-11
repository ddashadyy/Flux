#include "flpch.h"
#include "Application.h"

#include "Flux/Log.h"
#include "Flux/Core.h"

#include <GLFW/glfw3.h>

#include "Input.h"

namespace Flux {

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		s_Instance = this;

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(FL_BIND_EVENT_FN(Application::OnEvent));
	}

	Application::~Application()
	{
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(FL_BIND_EVENT_FN(Application::OnWindowClose));

		for (auto it = m_LayerStack.End(); it != m_LayerStack.Begin(); )
		{
			(*--it)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	void Application::Run()
	{
		while (m_Running)
		{

			auto [x, y] = Input::GetMousePosition();
			FL_CORE_TRACE("{0}, {1}", x, y);
			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

}