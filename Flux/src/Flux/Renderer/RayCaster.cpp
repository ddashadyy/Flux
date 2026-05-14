#include "flpch.h"
#include "RayCaster.h"

#include <glm/gtc/matrix_transform.hpp>

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
    int RayCaster::PickEntity(float ndcX, float ndcY,
        const PerspectiveCamera& camera,
        const Scene& scene)
    {
        Ray ray = BuildRay(ndcX, ndcY, camera);

        int   closestIndex = -1;
        float closestT = FLT_MAX;

        const auto& entities = scene.GetEntities();
        for (int i = 0; i < (int)entities.size(); ++i)
        {
            const Entity& entity = entities[i];
            auto model = entity.GetModel();
            if (!model || !model->Bounds.IsValid()) continue;

            glm::mat4 modelMatrix = entity.GetTransform().GetMatrix();
            float t = HitAABB(ray, model->Bounds, modelMatrix);

            if (t >= 0.0f && t < closestT)
            {
                closestT = t;
                closestIndex = i;
            }
        }

        return closestIndex;
    }

} // namespace Flux