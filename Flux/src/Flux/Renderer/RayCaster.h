#pragma once

#include "Flux/Renderer/Geometry.h"
#include "Flux/Renderer/PerspectiveCamera.h"
#include "Flux/Scene/Scene.h"

#include <glm/glm.hpp>

namespace Flux {

    struct Ray
    {
        glm::vec3 Origin;
        glm::vec3 Direction;
    };

    class RayCaster
    {
    public:
        static Ray BuildRay(float ndcX, float ndcY, const PerspectiveCamera& camera);

        static float HitAABB(const Ray& ray, const AABB& aabb, const glm::mat4& modelMatrix);

        static int PickEntity(float ndcX, float ndcY,
            const PerspectiveCamera& camera,
            const Scene& scene);
    };

} // namespace Flux