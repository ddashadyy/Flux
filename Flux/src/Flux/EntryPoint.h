#pragma once

#ifdef FL_PLATFORM_WINDOWS

extern Flux::Application* Flux::CreateApplication();

int main(int argc, char** argv)
{

	auto app = Flux::CreateApplication();
	app->Run();
	delete app;

	return 0;
}

#else 
	#error Flux only supports Windows now!
#endif
