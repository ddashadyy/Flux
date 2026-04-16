#pragma once


#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>   

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>   

#include "Flux/Renderer/Geometry.h"


namespace Flux {

    struct Transform
    {
        glm::vec3 Position = { 0, 0, 0 };
        glm::vec3 Rotation = { 0, 0, 0 }; // в градусах (углы Эйлера)
        glm::vec3 Scale = { 1, 1, 1 };

        glm::mat4 GetMatrix() const // Отдает матрицу модели
        {
            glm::mat4 t = glm::translate(glm::mat4(1.0f), Position);      // Перемещение
            glm::mat4 r = glm::toMat4(glm::quat(glm::radians(Rotation))); // Поворот
            glm::mat4 s = glm::scale(glm::mat4(1.0f), Scale);             // Масштаб 
            return t * r * s;
        }
    };


    class Entity
    {
    public:
        Entity(Ref<Mesh> mesh, Ref<Material> material);

        void Draw(RHICommandList& cmd) const;

        Transform& GetTransform() { return m_Transform; }
        const Transform& GetTransform() const { return m_Transform; }

        Ref<Mesh>     GetMesh()     const { return m_Mesh; }
        Ref<Material> GetMaterial() const { return m_Material; }

    private:
        Transform     m_Transform{};

        Ref<Mesh>     m_Mesh;
        Ref<Material> m_Material;
    };


}