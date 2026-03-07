#pragma once

#ifdef FL_PLATFORM_WINDOWS

extern Flux::Application* Flux::CreateApplication();

int main(int argc, char** argv)
{

	Flux::Log::Init();

	FL_CORE_WARN("asdasdas");
	FL_INFO("asdasdassadasdasdsad");
	 
	auto app = Flux::CreateApplication();
	app->Run();
	delete app;

	return 0;
}

#else 
	#error Flux only supports Windows now!
#endif
