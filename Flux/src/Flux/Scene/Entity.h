#pragma once

#include <entt/entt.hpp>

#include "Scene.h"
#include "Components.h"

namespace Flux {

    class Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene) : m_EntityHandle(handle), m_Scene(scene) {}
        Entity(const Entity&) = default;

        Scene* GetScene() { return m_Scene; }

        template<typename T, typename... Args>
        decltype(auto) AddComponent(Args&&... args)
        {
            return m_Scene->GetRegistry().emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
        }

        template<typename T>
        T& GetComponent()
        {
            return m_Scene->GetRegistry().get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent()
        {
            return m_Scene->GetRegistry().all_of<T>(m_EntityHandle);
        }

        template<typename T>
        void RemoveComponent()
        {
            m_Scene->GetRegistry().remove<T>(m_EntityHandle);
        }

        const std::string& GetTag() { return GetComponent<TagComponent>().Tag; }
        TransformComponent& GetTransform() { return GetComponent<TransformComponent>(); }

        operator bool() const { return m_EntityHandle != entt::null; }
        operator entt::entity() const { return m_EntityHandle; }

        bool operator==(const Entity& other) const
        {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }

    private:
        entt::entity m_EntityHandle{ entt::null };
        Scene* m_Scene = nullptr;
    };
}