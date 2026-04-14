#include "flpch.h"
#include "PerspectiveCamera.h"
#include "Flux/Core/Base.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Flux {

    PerspectiveCamera::PerspectiveCamera(float fov, float aspectRatio, float nearClip, float farClip)
        : Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip)),
        m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
    {
        m_Rotation = glm::vec3(0.0f, -90.0f, 0.0f);
        m_Position = glm::vec3(0.0f, 0.0f, 3.0f);
        RecalculateViewMatrix();
    }

    void PerspectiveCamera::SetPosition(const glm::vec3& position)
    {
        m_Position = position;
        RecalculateViewMatrix();
    }

    void PerspectiveCamera::SetRotation(const glm::vec3& rotation)
    {
        m_Rotation = rotation;
        RecalculateViewMatrix();
    }

    void PerspectiveCamera::RecalculateViewMatrix()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(m_Rotation.y)) * cos(glm::radians(m_Rotation.x));
        front.y = sin(glm::radians(m_Rotation.x));
        front.z = sin(glm::radians(m_Rotation.y)) * cos(glm::radians(m_Rotation.x));
        front = glm::normalize(front);

        m_ViewMatrix = glm::lookAt(m_Position, m_Position + front, glm::vec3(0.0f, 1.0f, 0.0f));
        m_ViewProjectionMatrix = GetProjection() * m_ViewMatrix;
    }

} 