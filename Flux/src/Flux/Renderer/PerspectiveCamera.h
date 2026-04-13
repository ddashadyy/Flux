#pragma once

#include "Camera.h"
#include <glm/glm.hpp>

namespace Flux {

    class PerspectiveCamera : public Camera
    {
    public:
        PerspectiveCamera(float fov, float aspectRatio, float nearClip, float farClip);
        virtual ~PerspectiveCamera() = default;

        void SetPosition(const glm::vec3& position);
        void SetRotation(const glm::vec3& rotation); // pitch, yaw, roll в градусах

        inline const glm::vec3& GetPosition() const { return m_Position; }
        inline const glm::vec3& GetRotation() const { return m_Rotation; }

		inline const float GetFOV() const { return m_FOV; }
		inline void SetFOV(float fov) { m_FOV = fov; }
		inline void SetAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; }

        inline const glm::mat4& GetViewMatrix()           const { return m_ViewMatrix; }
        inline const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

    private:
        void RecalculateViewMatrix();

    private:
        float m_FOV = 0.45f;
        float m_AspectRatio = 1.778f;
        float m_NearClip = 0.1f;
        float m_FarClip = 1000.0f;

        glm::vec3 m_Position = { 0.0f, 0.0f, 3.0f };
        glm::vec3 m_Rotation = { 0.0f, -90.0f, 0.0f }; // pitch, yaw, roll

        glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
        glm::mat4 m_ViewProjectionMatrix = glm::mat4(1.0f);
    };

}