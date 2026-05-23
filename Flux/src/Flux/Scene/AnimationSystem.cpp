#include "flpch.h"
#include "AnimationSystem.h"
#include "Components.h"


namespace Flux {

    void AnimationSystem::Update(Scene& scene, float dt)
    {
        auto view = scene.GetRegistry().view<MeshComponent, AnimatorComponent>();

        for (auto entity : view)
        {
            auto& mesh = view.get<MeshComponent>(entity);
            auto& anim = view.get<AnimatorComponent>(entity);

            if (!anim.Playing || !anim.CurrentClip || !mesh.Model || !mesh.Model->Skel)
                continue;

            anim.CurrentTime += dt * anim.Speed;
            if (anim.Loop)
                anim.CurrentTime = fmod(anim.CurrentTime, anim.CurrentClip->Duration);
            else
                anim.CurrentTime = std::min(anim.CurrentTime, anim.CurrentClip->Duration);

            EvaluateClip(anim, *mesh.Model->Skel);

            if (anim.SkinningBuffer)
            {
                size_t size = anim.JointMatrices.size() * sizeof(glm::mat4);
                anim.SkinningBuffer->SetData(anim.JointMatrices.data(), size);
            }
        }
    }

    void AnimationSystem::EvaluateClip(AnimatorComponent& anim, const Skeleton& skeleton)
    {
        auto& clip = *anim.CurrentClip;
        int   n = (int)skeleton.Joints.size();

        // local TRS
        std::vector<glm::mat4> localMatrices(n);
        for (int i = 0; i < n; i++)
        {
            glm::vec3 pos = clip.PositionChannels[i].Sample(anim.CurrentTime);
            glm::quat rot = clip.RotationChannels[i].Sample(anim.CurrentTime);
            glm::vec3 scale = clip.ScaleChannels[i].Sample(anim.CurrentTime);

            if (clip.PositionChannels[i].Times.empty()) pos = glm::vec3(0.f);
            if (clip.RotationChannels[i].Times.empty()) rot = glm::quat(1, 0, 0, 0);
            if (clip.ScaleChannels[i].Times.empty())    scale = glm::vec3(1.f);

            localMatrices[i] = glm::translate(glm::mat4(1.f), pos)
                * glm::mat4_cast(rot)
                * glm::scale(glm::mat4(1.f), scale);
        }

        std::vector<glm::mat4> globalMatrices(n);
        for (int i = 0; i < n; i++)
        {
            int p = skeleton.Joints[i].ParentIndex;
            globalMatrices[i] = (p < 0)
                ? localMatrices[i]
                : globalMatrices[p] * localMatrices[i];
        }

        anim.JointMatrices.resize(n);
        for (int i = 0; i < n; i++)
            anim.JointMatrices[i] = globalMatrices[i] * skeleton.Joints[i].InverseBindMatrix;
    }

}