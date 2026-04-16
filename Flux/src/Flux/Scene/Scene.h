#pragma once

#include "Flux/Renderer/Geometry.h"
#include "Flux/Renderer/RHICommandList.h"

#include "Entity.h"

namespace Flux {

    

    class Scene
    {
    public:
		using Entities = std::vector<Entity>;

        Scene() = default;

        Entity& AddEntity(Ref<Mesh> mesh, Ref<Material> material);
        void Draw(RHICommandList& cmd) const;

        const Entities& GetEntities() const { return m_Entities; }

    private:
        Entities m_Entities;
    };

}