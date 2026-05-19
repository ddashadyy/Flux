#include "flpch.h"
#include "RayCaster.h"
#include "Flux/Scene/Entity.h"
#include "Flux/Scene/Components.h" 

#include <glm/gtc/matrix_transform.hpp>
#include <limits>

namespace Flux {

    // -------------------------------------------------------------------------
    // BuildRay
    // -------------------------------------------------------------------------
    Ray RayCaster::BuildRay(float ndcX, float ndcY, const PerspectiveCamera& camera)
    {
        glm::mat4 invVP = glm::inverse(camera.GetViewProjectionMatrix());

        glm::vec4 worldNear = invVP * glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
        worldNear /= worldNear.w;

        glm::vec4 worldFar = invVP * glm::vec4(ndcX, ndcY, 1.0f, 1.0f);
        worldFar /= worldFar.w;

        Ray ray;
        ray.Origin = camera.GetPosition();
        ray.Direction = glm::normalize(glm::vec3(worldFar) - glm::vec3(worldNear));
        return ray;
    }

    // -------------------------------------------------------------------------
    // HitAABB — slab method в object space
    // -------------------------------------------------------------------------
    float RayCaster::HitAABB(const Ray& ray, const AABB& aabb, const glm::mat4& modelMatrix)
    {
        if (!aabb.IsValid()) return -1.0f;

        glm::mat4 invModel = glm::inverse(modelMatrix);

        glm::vec3 rayOriginOS = glm::vec3(invModel * glm::vec4(ray.Origin, 1.0f));
        glm::vec3 rayDirectionOS = glm::vec3(invModel * glm::vec4(ray.Direction, 0.0f));

        glm::vec3 invDir = 1.0f / rayDirectionOS;

        glm::vec3 t0 = (aabb.Min - rayOriginOS) * invDir;
        glm::vec3 t1 = (aabb.Max - rayOriginOS) * invDir;

        glm::vec3 tMin = glm::min(t0, t1);
        glm::vec3 tMax = glm::max(t0, t1);

        float tEnter = glm::max(glm::max(tMin.x, tMin.y), tMin.z);
        float tExit = glm::min(glm::min(tMax.x, tMax.y), tMax.z);

        if (tExit < 0.0f || tEnter > tExit)
            return -1.0f;

        return tEnter >= 0.0f ? tEnter : tExit;
    }

    // -------------------------------------------------------------------------
    // PickEntity
    // -------------------------------------------------------------------------
    Entity RayCaster::PickEntity(float ndcX, float ndcY,
        const PerspectiveCamera& camera,
        Scene& scene)
    {
        Ray ray = BuildRay(ndcX, ndcY, camera);

        Entity closestEntity;
        float closestT = std::numeric_limits<float>::max();

        auto& registry = scene.GetRegistry();
        auto view = registry.view<TransformComponent, MeshComponent>();

        for (auto entityHandle : view)
        {
            auto& transform = view.get<TransformComponent>(entityHandle);
            auto& meshComp = view.get<MeshComponent>(entityHandle);

            if (!meshComp.Model || !meshComp.Model->Bounds.IsValid()) continue;

            glm::mat4 modelMatrix = transform.GetLocalMatrix();
            float t = HitAABB(ray, meshComp.Model->Bounds, modelMatrix);

            if (t >= 0.0f && t < closestT)
            {
                closestT = t;
                closestEntity = Entity(entityHandle, &scene);
            }
        }

        return closestEntity;
    }

} // namespace Flux