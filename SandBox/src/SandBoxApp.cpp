#include <Flux.h>

#include "imgui/imgui.h"


class ExampleLayer : public Flux::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
		FL_INFO("Creating shader...");
		m_Shader = Flux::Shader::Create("C:/dev/Flux/Sandbox/assets/shaders/shader");
		FL_INFO("Creating pipeline...");
		m_Pipeline = Flux::Pipeline::Create(m_Shader);
		FL_INFO("ExampleLayer ready");
	}

	void OnUpdate() override
	{
		auto& context = Flux::VulkanContext::Get();
		VkCommandBuffer cmd = context.GetCurrentCommandBuffer();

		m_Pipeline->Bind();
		vkCmdDraw(cmd, 3, 1, 0, 0);
	}

	virtual void OnImGuiRender() override
	{
		//ImGui::Begin("Test");
		//ImGui::Text("Hello World");
		//ImGui::End();
	}

	void OnEvent(Flux::Event& event) override
	{
		if (event.GetEventType() == Flux::EventType::KeyPressed)
		{
			Flux::KeyPressedEvent& e = static_cast<Flux::KeyPressedEvent&>(event);
			FL_TRACE("Key: {0}", static_cast<char>(e.GetKeyCode()));
		}
	}

private: 
	Flux::Ref<Flux::Shader>   m_Shader;
	Flux::Ref<Flux::Pipeline> m_Pipeline;
};

class Sandbox : public Flux::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
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