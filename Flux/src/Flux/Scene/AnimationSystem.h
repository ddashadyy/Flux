#pragma once

#include "Scene.h"
#include "Components.h"

namespace Flux {

    class AnimationSystem
    {
    public:
        static void Update(Scene& scene, float dt);

    private:
        static void EvaluateClip(AnimatorComponent& anim, const Skeleton& skeleton);
    };

}