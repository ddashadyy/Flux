#include <Flux.h>

#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

class ExampleLayer : public Flux::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
		std::vector<Flux::Vertex> vertices = {
			{{ 0.5f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // 0: Верхний правый (Красный)
			{{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // 1: Нижний правый (Зеленый)
			{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // 2: Нижний левый (Синий)
			{{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}  // 3: Верхний левый (Белый)
		};

		std::vector<uint32_t> indices = {
			0, 1, 2,
			2, 3, 0
		};

		m_Shader = Flux::Shader::Create("C:/dev/Flux/Sandbox/assets/shaders/shader");
		
		Flux::BufferLayout layout = {
			{ Flux::ShaderDataType::Float3, "Position" },
			{ Flux::ShaderDataType::Float3, "Color"    },
		};

		m_Pipeline = Flux::Pipeline::Create(m_Shader, layout);
		
		m_VertexBuffer = Flux::VertexBuffer::Create(
			reinterpret_cast<float*>(vertices.data()),
			sizeof(Flux::Vertex) * vertices.size()
		);
		m_VertexBuffer->SetLayout(layout);

		m_IndexBuffer = Flux::IndexBuffer::Create(
			indices.data(),
			sizeof(uint32_t) * indices.size()
		);

		m_UniformBuffer = Flux::UniformBuffer::Create(sizeof(Flux::UniformBufferObject));
		m_Pipeline->SetUniformBuffer(m_UniformBuffer);

		m_Camera = Flux::CreateRef<Flux::PerspectiveCamera>(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
		m_Camera->SetPosition({ 0.0f, 0.0f, 2.0f });
		m_Camera->SetRotation({ 0.0f, -90.0f, 0.0f });
	}

	void OnUpdate() override
	{
		Flux::UniformBufferObject ubo{};

		ubo.Model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
		ubo.Model *= glm::rotate(glm::mat4(1.0f), static_cast<float>(glfwGetTime()), glm::vec3(1.0f, 0.0f, 0.0f));

		ubo.View = m_Camera->GetViewMatrix();

		ubo.Projection = m_Camera->GetProjection();
		ubo.Projection[1][1] *= -1;

		m_UniformBuffer->SetData(ubo);

		Flux::Renderer::Submit(m_Pipeline, m_VertexBuffer, m_IndexBuffer);
	}

	virtual void OnImGuiRender() override
	{
		ImGui::Text("Renderer API: %s", Flux::RendererAPI::GetAPIName());
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
	Flux::Ref<Flux::Shader>        m_Shader;
	Flux::Ref<Flux::Pipeline>      m_Pipeline;
	Flux::Ref<Flux::VertexBuffer>  m_VertexBuffer;
	Flux::Ref<Flux::IndexBuffer>   m_IndexBuffer;
	Flux::Ref<Flux::UniformBuffer> m_UniformBuffer;
	Flux::Ref<Flux::PerspectiveCamera> m_Camera;
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