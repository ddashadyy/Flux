#pragma once

#include "Geometry.h"
#include "RHITexture.h"
#include "RHIDevice.h"

// проблема: модели имеют не один материал и текстуры
// решение: сабмэш, который описывает одну часть модели (один материал)

namespace Flux {

    using Texture = RHITexture;
    
    struct SubMesh
    {
        Scope<RHIBuffer> IndexBuffer;
        uint32_t IndexCount = 0;
        Scope<RHIDescriptorSet> DescriptorSet; // для текстур данного сабмэша
    };

    struct Model
    {
        Scope<RHIBuffer> VertexBuffer; // данный буфер общий для всех
        std::vector<SubMesh> Meshes;   // список частей
    };

    class AssetManager
    {
    public:
        template<typename Hash, typename T>
        using CacheMap = std::unordered_map<Hash, Ref<T>>;

        explicit AssetManager(RHIDevice& device);

        Ref<Model>     LoadModel(const std::string& path, RHIDescriptorSetLayout* textureLayout);
        Ref<Texture>  LoadTexture(const std::string& path);

        void ClearCache();

    private:
        Ref<Model>     LoadModelFromFile(const std::string& path, RHIDescriptorSetLayout* textureLayout);
        Ref<Texture>  LoadTextureFromFile(const std::string& path);

    private:
        RHIDevice& m_Device;

        CacheMap<std::string, Model>     m_ModelCache;
        CacheMap<std::string, Texture>   m_TextureCache;

        Ref<Texture> m_DefaultWhiteTexture;
        Ref<Texture> m_DefaultNormalTexture;
    };

}