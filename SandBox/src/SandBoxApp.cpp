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
		//FL_INFO("ExampleLayer::Update");
		if (Flux::Input::IsKeyPressed(FL_KEY_TAB))
			FL_TRACE("Tab key is pressed");
	}

	void OnEvent(Flux::Event& event) override
	{
		//FL_TRACE("{0}", event);  // выводи все события

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
		m_ImGuiLayer = new Flux::ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	~Sandbox()
	{
		PopOverlay(m_ImGuiLayer);
	}
private:
	Flux::ImGuiLayer* m_ImGuiLayer = nullptr;

};

Flux::Application* Flux::CreateApplication()
{
	return new Sandbox();
}