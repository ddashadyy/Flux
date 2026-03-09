#include "flpch.h"
#include "Application.h"

#include "Flux/Events/ApplicationEvent.h"
#include "Flux/Log.h"

#include <GLFW/glfw3.h>

namespace Flux {

	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create());
	}


	Application::~Application()
	{
	}

	void Application::Run()
	{
		
		while (m_Running)
		{	
			m_Window->OnUpdate();
		}
		
	}

}