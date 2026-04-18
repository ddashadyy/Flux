#pragma once

#include "Entity.h"
#include "Flux/Renderer/RHICommandList.h"

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

        void RemoveEntity(size_t index)
        {
            if (index < m_Entities.size())
                m_Entities.erase(m_Entities.begin() + index);
        }

        Entities& GetEntities() { return m_Entities; }
        const Entities& GetEntities() const { return m_Entities; }

    private:
        Entities m_Entities;
    };
}