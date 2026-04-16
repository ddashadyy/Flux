#include "flpch.h"
#include "Scene.h"

namespace Flux {

    Entity& Scene::AddEntity(Ref<Mesh> mesh, Ref<Material> material)
    {
        return m_Entities.emplace_back(std::move(mesh), std::move(material));
    }

    void Scene::Draw(RHICommandList& cmd) const
    {
        for (const auto& entity : m_Entities)
            entity.Draw(cmd);
    }

}