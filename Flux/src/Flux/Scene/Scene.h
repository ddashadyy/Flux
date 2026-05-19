#pragma once


#include <entt/entt.hpp>
#include "Flux/Core/UUID.h"

namespace Flux {

    class Entity;

    class Scene
    {
    public:
        Scene() = default;

        Entity CreateEntity(const std::string& name = "Entity");
        Entity CreateEntityWithID(UUID uuid, const std::string& name = "Entity");

        void DestroyEntity(Entity entity); 
        void ProcessDeletions();           

        void OnUpdate(float dt);

        entt::registry& GetRegistry() { return m_Registry; }
        const entt::registry& GetRegistry() const { return m_Registry; }

        Entity GetEntityByUUID(UUID uuid);
        void SetParent(Entity child, Entity parent);

    private:
        entt::registry m_Registry;
        std::unordered_map<UUID, entt::entity> m_EntityMap;
    };
}
