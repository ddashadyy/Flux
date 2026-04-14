#pragma once

#ifdef FL_PLATFORM_WINDOWS

extern Flux::Application* Flux::CreateApplication();

int main(int argc, char** argv)
{

	Flux::Log::Init();

	auto app = Flux::CreateApplication();
	app->Run();
	delete app;

	return 0;
}

#endif
