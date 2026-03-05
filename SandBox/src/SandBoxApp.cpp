#include <Flux.h>

class SandBox final : public Flux::Application
{
public:
	SandBox() {}
	~SandBox() {}

};


Flux::Application* Flux::CreateApplication()
{
	return new SandBox();
}