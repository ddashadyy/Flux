#pragma once

#include "Entity.h"
#include "Flux/Renderer/RHICommandList.h"

#include <algorithm>

namespace Flux {

    class Scene
    {
    public:
        using Entities = std::vector<Entity>;

        Scene() = default;

        Entity& AddEntity(Ref<Model> model, const std::string& name = "")
        {
            auto& e = m_Entities.emplace_back(std::move(model));
            if (!name.empty()) e.SetName(name);
            return e;
        }

        Entity& DuplicateEntity(size_t index)
        {
            if (index >= m_Entities.size())
                return m_Entities.back();

            Entity newEntity = m_Entities[index];

            newEntity.GetTransform().Position.x += 1.0f;

            if (!newEntity.GetName().empty())
                newEntity.SetName(newEntity.GetName() + " Copy");

            m_Entities.emplace_back(newEntity);
            return m_Entities.back();
        }

        void RemoveEntity(size_t index)
        {
            if (index < m_Entities.size())
            {
                m_Entities[index].MarkForDeletion();
            }
        }

        void ProcessDeletions()
        {
            std::erase_if(m_Entities, [](const Entity& e) { return e.IsMarkedForDeletion(); });
        }

        Entities& GetEntities() { return m_Entities; }
        const Entities& GetEntities() const { return m_Entities; }

    private:
        Entities m_Entities;
    };
}