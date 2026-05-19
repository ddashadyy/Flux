#include "flpch.h"
#include "Scene.h"
#include "Entity.h"

#include <algorithm> 

namespace Flux {

    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity entity = { m_Registry.create(), this };
        auto& id = entity.AddComponent<IDComponent>();
        entity.AddComponent<TagComponent>(name);
        entity.AddComponent<TransformComponent>();
        m_EntityMap[id.ID] = entity;
        return entity;
    }

    Entity Scene::CreateEntityWithID(UUID uuid, const std::string& name)
    {
        Entity entity = { m_Registry.create(), this };
        auto& id = entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TagComponent>(name);
        entity.AddComponent<TransformComponent>();
        m_EntityMap[id.ID] = entity;
        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        if (!entity || entity.HasComponent<DestroyFlag>()) return;

        entity.AddComponent<DestroyFlag>();

        if (entity.HasComponent<RelationshipComponent>())
        {
            auto& rel = entity.GetComponent<RelationshipComponent>();

            auto childrenCopy = rel.Children;
            for (UUID childID : childrenCopy)
            {
                Entity child = GetEntityByUUID(childID);
                if (child) 
                    child.GetComponent<RelationshipComponent>().ParentID = UUID(nullptr);
            }

            if (rel.ParentID != UUID(nullptr))
            {
                Entity parent = GetEntityByUUID(rel.ParentID);
                if (parent && parent.HasComponent<RelationshipComponent>())
                {
                    auto& parentRel = parent.GetComponent<RelationshipComponent>();
                    UUID entityID = entity.GetComponent<IDComponent>().ID;

                    auto it = std::remove(parentRel.Children.begin(), parentRel.Children.end(), entityID);
                    if (it != parentRel.Children.end()) 
                        parentRel.Children.erase(it, parentRel.Children.end());
                }
            }
        }
    }

    void Scene::ProcessDeletions()
    {
        auto view = m_Registry.view<DestroyFlag>();

        for (auto entity : view)
        {
            Entity e(entity, this);

            if (e.HasComponent<IDComponent>())
                m_EntityMap.erase(e.GetComponent<IDComponent>().ID);

            m_Registry.destroy(entity);
        }
    }

    Entity Scene::GetEntityByUUID(UUID uuid)
    {
        auto it = m_EntityMap.find(uuid);
        if (it != m_EntityMap.end()) return Entity(it->second, this);
        return Entity();
    }

    void Scene::SetParent(Entity child, Entity parent)
    {
        if (!child || !parent || child == parent) return;

        auto& childRel = child.HasComponent<RelationshipComponent>() ?
            child.GetComponent<RelationshipComponent>() : child.AddComponent<RelationshipComponent>();

        auto& parentRel = parent.HasComponent<RelationshipComponent>() ?
            parent.GetComponent<RelationshipComponent>() : parent.AddComponent<RelationshipComponent>();

        if (childRel.ParentID != UUID(nullptr)) 
        {
            Entity oldParent = GetEntityByUUID(childRel.ParentID);
            if (oldParent) 
            {
                auto& oldRel = oldParent.GetComponent<RelationshipComponent>();
                UUID childID = child.GetComponent<IDComponent>().ID;

                auto it = std::remove(oldRel.Children.begin(), oldRel.Children.end(), childID);
                if (it != oldRel.Children.end()) 
                    oldRel.Children.erase(it, oldRel.Children.end());
                
            }
        }

        childRel.ParentID = parent.GetComponent<IDComponent>().ID;
        parentRel.Children.push_back(child.GetComponent<IDComponent>().ID);
    }

    static void UpdateTransformRecursive(Entity entity)
    {
        auto& transform = entity.GetComponent<TransformComponent>();
        auto& rel = entity.GetComponent<RelationshipComponent>();

        if (rel.ParentID == UUID(nullptr)) 
            transform.WorldMatrix = transform.GetLocalMatrix();
        
        else 
        {
            Entity parent = entity.GetScene()->GetEntityByUUID(rel.ParentID);
            if (parent)
            {
                auto& parentTransform = parent.GetComponent<TransformComponent>();
                transform.WorldMatrix = parentTransform.WorldMatrix * transform.GetLocalMatrix();
            }
            else
                transform.WorldMatrix = transform.GetLocalMatrix();
        }

        for (UUID childID : rel.Children) 
        {
            Entity child = entity.GetScene()->GetEntityByUUID(childID);
            if (child) UpdateTransformRecursive(child);
        }
    }

    void Scene::OnUpdate(float dt)
    {
        auto simpleView = m_Registry.view<TransformComponent>(entt::exclude<RelationshipComponent, DestroyFlag>);
        for (auto entity : simpleView) 
        {
            simpleView.get<TransformComponent>(entity).WorldMatrix = simpleView.get<TransformComponent>(entity).GetLocalMatrix();
        }

        auto view = m_Registry.view<TransformComponent, RelationshipComponent>(entt::exclude<DestroyFlag>);
        for (auto entity : view) 
        {
            if (view.get<RelationshipComponent>(entity).ParentID == UUID(nullptr)) 
            { 
                UpdateTransformRecursive(Entity(entity, this));
            }
        }
    }
}