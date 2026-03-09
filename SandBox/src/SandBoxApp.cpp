#include <Flux.h>

class ExampleLayer : public Flux::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		FL_INFO("ExampleLayer::Update");
	}

	void OnEvent(Flux::Event& event) override
	{
		FL_TRACE("{0}", event);
	}

};

class Sandbox : public Flux::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new Flux::ImGuiLayer());
	}

	~Sandbox()
	{

	}

};

Flux::Application* Flux::CreateApplication()
{
	return new Sandbox();
}