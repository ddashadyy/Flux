#include "flpch.h"

#include "Application.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"

namespace Flux {

	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		WindowResizeEvent e(1280, 720);
		FL_TRACE(e);

		while (true);
	}

} // namespace Flux