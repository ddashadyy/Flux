#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <optional>
#include <string>

#include "Flux/Renderer/RHICommandList.h"
#include "Flux/Renderer/AssetManager.h" 

namespace Flux {

    struct Transform
    {
        glm::vec3 Position = { 0, 0, 0 };
        glm::vec3 Rotation = { 0, 0, 0 };
        glm::vec3 Scale = { 1, 1, 1 };

        glm::mat4 GetMatrix() const
        {
            glm::mat4 t = glm::translate(glm::mat4(1.0f), Position);
            glm::mat4 r = glm::toMat4(glm::quat(glm::radians(Rotation)));
            glm::mat4 s = glm::scale(glm::mat4(1.0f), Scale);
            return t * r * s;
        }
    };

    struct LightComponent
    {
        enum class LightType { Point, Directional, Spot } Type = LightType::Point;
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
        float Intensity = 1.0f;
        float Range = 10.0f;
    };

    class Entity
    {
    public:
        Entity() = default;
        explicit Entity(Ref<Model> model) : m_Model(std::move(model)) {}

        Transform& GetTransform() { return m_Transform; }
        const Transform& GetTransform() const { return m_Transform; }

        Ref<Model> GetModel() const { return m_Model; }
        void       SetModel(Ref<Model> model) { m_Model = std::move(model); }

        const std::string& GetName() const { return m_Name; }
        void               SetName(const std::string& name) { m_Name = name; }


        void MarkForDeletion()           { m_MarkedForDeletion = true; }
        bool IsMarkedForDeletion() const { return m_MarkedForDeletion; }

        // --- МЕТОДЫ ДЛЯ РАБОТЫ С КОМПОНЕНТАМИ ---
        bool                  HasLight() const { return m_Light.has_value(); }

        LightComponent&       GetLight() { return m_Light.value(); }
        const LightComponent& GetLight() const { return m_Light.value(); }

        void AddLight()    { m_Light.emplace(); }
        void RemoveLight() { m_Light.reset(); }

    private:
        std::string m_Name;
        Transform   m_Transform{};
        Ref<Model>  m_Model;

        std::optional<LightComponent> m_Light;

        bool m_MarkedForDeletion = false;
    };
}