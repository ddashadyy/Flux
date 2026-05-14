#include "flpch.h"
#include "EditorCamera.h"

#include "Flux/Core/Application.h"
#include "Flux/Core/Input.h"
#include "Flux/Core/KeyCodes.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace Flux {

    EditorCamera::EditorCamera(float fov, float aspect, float nearP, float farP)
        : m_Camera(fov, aspect, nearP, farP)
    {
    }

    void EditorCamera::OnUpdate(float dt)
    {
        ProcessKeyboard(dt);
    }

    void EditorCamera::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseMovedEvent>(FL_BIND_EVENT_FN(EditorCamera::OnMouseMoved));
        dispatcher.Dispatch<MouseScrolledEvent>(FL_BIND_EVENT_FN(EditorCamera::OnMouseScrolled));
        dispatcher.Dispatch<KeyPressedEvent>(FL_BIND_EVENT_FN(EditorCamera::OnKeyPressed));
    }

    void EditorCamera::ProcessKeyboard(float dt)
    {
        if (!m_CursorLocked) return;

        float speed = m_Speed * dt;
        glm::vec3 move(0.0f);

        if (Input::IsKeyPressed(FL_KEY_W))          move.z += 1.0f;
        if (Input::IsKeyPressed(FL_KEY_S))          move.z -= 1.0f;
        if (Input::IsKeyPressed(FL_KEY_A))          move.x -= 1.0f;
        if (Input::IsKeyPressed(FL_KEY_D))          move.x += 1.0f;
        if (Input::IsKeyPressed(FL_KEY_SPACE))      move.y += 1.0f;
        if (Input::IsKeyPressed(FL_KEY_LEFT_SHIFT)) move.y -= 1.0f;

        if (glm::length(move) > 0.0f)
            move = glm::normalize(move);

        glm::vec3 pos = m_Camera.GetPosition();
        pos += m_Camera.GetForward() * move.z * speed;
        pos += m_Camera.GetRight() * move.x * speed;
        pos += glm::vec3(0, 1, 0) * move.y * speed;
        m_Camera.SetPosition(pos);
    }

    bool EditorCamera::OnMouseMoved(MouseMovedEvent& e)
    {
        if (!m_CursorLocked) return false;

        if (m_FirstMouse)
        {
            m_LastX = e.GetX(); m_LastY = e.GetY();
            m_FirstMouse = false;
            return false;
        }

        float xOff = (e.GetX() - m_LastX) * m_Sensitivity;
        float yOff = (m_LastY - e.GetY()) * m_Sensitivity;
        m_LastX = e.GetX();
        m_LastY = e.GetY();

        glm::vec3 rot = m_Camera.GetRotation();
        rot.y += xOff;
        rot.x = glm::clamp(rot.x + yOff, -89.0f, 89.0f);
        m_Camera.SetRotation(rot);
        return false;
    }

    bool EditorCamera::OnMouseScrolled(MouseScrolledEvent& e)
    {
        m_Camera.SetFOV(glm::clamp(m_Camera.GetFOV() - e.GetYOffset(), 1.0f, 90.0f));
        return false;
    }

    bool EditorCamera::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetKeyCode() == FL_KEY_ESCAPE)
        {
            m_CursorLocked = !m_CursorLocked;
            SetCursorMode(m_CursorLocked);
        }

        if (m_KeyCallback)
            m_KeyCallback(e.GetKeyCode());

        return false;
    }

    void EditorCamera::SetCursorMode(bool locked)
    {
        GLFWwindow* w = static_cast<GLFWwindow*>(
            Application::Get().GetWindow().GetNativeWindow());

        glfwSetInputMode(w, GLFW_CURSOR,
            locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

        if (locked) m_FirstMouse = true;
    }

} // namespace Flux