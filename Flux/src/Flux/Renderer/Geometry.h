#pragma once


#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "RHIBuffer.h"
#include "RHIDevice.h"
#include "RHIShader.h"
#include "RHITexture.h"

#include <filesystem>

namespace Flux {

    struct AABB
    {
        glm::vec3 Min{ FLT_MAX,  FLT_MAX,  FLT_MAX };
        glm::vec3 Max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

        glm::vec3 Center()  const { return (Min + Max) * 0.5f; }
        glm::vec3 Extents() const { return (Max - Min) * 0.5f; }
        bool      IsValid() const { return Min.x <= Max.x; }
    };

    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoord;
        glm::vec3 Tangent;   

        bool operator == (const Vertex& other) const
        {
            return Position == other.Position &&
                Normal == other.Normal &&
                TexCoord == other.TexCoord &&
                Tangent == other.Tangent;  
        }
    };

    struct SkinnedVertex
    {
        glm::vec3  Position;
        glm::vec3  Normal;
        glm::vec2  TexCoord;
        glm::vec3  Tangent;
        glm::uvec4 JointIndices{ 0, 0, 0, 0 };
        glm::vec4  JointWeights{ 1, 0, 0, 0 };
    };

    struct Joint
    {
        std::string Name;
        int32_t     ParentIndex = -1;      // -1 = root
        glm::mat4   InverseBindMatrix{ 1.0f };
    };

    struct Skeleton
    {
        std::vector<Joint> Joints;         

        int FindJoint(const std::string& name) const
        {
            for (int i = 0; i < (int)Joints.size(); i++)
                if (Joints[i].Name == name) return i;
            return -1;
        }
    };

    template<typename T>
    struct AnimationChannel
    {
        std::vector<float> Times;
        std::vector<T>     Values;

        T Sample(float t) const
        {
            if (Times.empty()) return T{};
            if (t <= Times.front()) return Values.front();
            if (t >= Times.back())  return Values.back();

            auto it = std::upper_bound(Times.begin(), Times.end(), t);
            int hi = (int)std::distance(Times.begin(), it);
            int lo = hi - 1;

            float alpha = (t - Times[lo]) / (Times[hi] - Times[lo]);

            if constexpr (std::is_same_v<T, glm::quat>)
                return glm::normalize(glm::slerp(Values[lo], Values[hi], alpha));
            else
                return glm::mix(Values[lo], Values[hi], alpha);
        }
    };

    struct AnimationClip
    {
        std::string Name;
        float       Duration = 0.f;

        // per-joint каналы
        std::vector<AnimationChannel<glm::vec3>> PositionChannels;
        std::vector<AnimationChannel<glm::quat>> RotationChannels;
        std::vector<AnimationChannel<glm::vec3>> ScaleChannels;
    };

    using Texture = RHITexture;

    struct Material
    {
        Ref<Texture> Albedo;
        Ref<Texture> Normal;
        Ref<Texture> RoughnessMetallic;

        glm::vec4 Color = glm::vec4(1.0f);
        // -1 == брать из текстуры
        float     RoughnessOverride = -1.0f;
        float     MetallicOverride = -1.0f;

        Scope<RHIDescriptorSet> DescriptorSet;
    };

    struct SubMesh
    {
        Scope<RHIBuffer> IndexBuffer;
        uint32_t         IndexCount = 0;
        uint32_t         BaseVertex = 0; 
        IndexType        Type = IndexType::Uint32;
        Material         Mat;

        void Draw(RHICommandList& cmdList) const
        {
            cmdList.BindIndexBuffer(IndexBuffer.get(), Type);
            cmdList.DrawIndexed(IndexCount, 1, 0, BaseVertex, 0);
        }
    };

    struct Model
    {
        std::filesystem::path Path;
        Scope<RHIBuffer>     VertexBuffer;
        std::vector<SubMesh> Meshes;
        AABB                 Bounds;

        // Skinning
        bool                           IsSkinned = false;
        Ref<Skeleton>                  Skel;
        std::vector<SkinnedVertex>     SkinnedVertices;
        std::vector<Ref<AnimationClip>> Animations;

        const std::filesystem::path& GetPath() const { return Path; }
    };

}