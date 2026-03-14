#include <Flux.h>

#include "imgui/imgui.h"

class ExampleLayer : public Flux::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
	}

	void OnUpdate() override
	{
		//FL_INFO("ExampleLayer::Update");
		if (Flux::Input::IsKeyPressed(FL_KEY_TAB))
			FL_TRACE("Tab key is pressed");
	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Test");
		ImGui::Text("Hello World");
		ImGui::End();
	}

	void OnEvent(Flux::Event& event) override
	{

		if (event.GetEventType() == Flux::EventType::KeyPressed)
		{
			Flux::KeyPressedEvent& e = static_cast<Flux::KeyPressedEvent&>(event);
			FL_TRACE("Key: {0}", static_cast<char>(e.GetKeyCode()));
		}
	}

};

class Sandbox : public Flux::Application
{
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		//PushOverlay(new Flux::ImGuiLayer());
	}

	~Sandbox()
	{
	}
private:
	
};

Flux::Application* Flux::CreateApplication()
{
	return new Sandbox();
}