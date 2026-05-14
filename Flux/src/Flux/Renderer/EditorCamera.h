#pragma once

#include "Flux/Renderer/PerspectiveCamera.h"
#include "Flux/Events/MouseEvent.h"
#include "Flux/Events/KeyEvent.h"

namespace Flux {

    class EditorCamera
    {
    public:
        using KeyCallback = std::function<void(int keyCode)>;


        EditorCamera() = default;
        EditorCamera(float fov, float aspect, float nearP, float farP);

        void OnUpdate(float dt);
        void OnEvent(Event& e);

        PerspectiveCamera& GetCamera() { return m_Camera; }
        const PerspectiveCamera& GetCamera() const { return m_Camera; }

        void SetViewportFocused(bool focused) { m_ViewportFocused = focused; }
        void SetKeyCallback(KeyCallback cb) { m_KeyCallback = std::move(cb); }

        bool IsCursorLocked() const { return m_CursorLocked; }

    private:
        void ProcessKeyboard(float dt);
        bool OnMouseMoved(MouseMovedEvent& e);
        bool OnMouseScrolled(MouseScrolledEvent& e);
        bool OnKeyPressed(KeyPressedEvent& e);
        void SetCursorMode(bool locked);

    private:
        PerspectiveCamera m_Camera;

        KeyCallback m_KeyCallback;

        float m_Speed = 5.0f;
        float m_Sensitivity = 0.1f;
        float m_LastX = 0.0f;
        float m_LastY = 0.0f;
        bool  m_FirstMouse = true;
        bool  m_CursorLocked = true;
        bool  m_ViewportFocused = false;
    };

} // namespace Flux