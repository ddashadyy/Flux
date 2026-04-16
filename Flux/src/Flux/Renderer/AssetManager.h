#pragma once

#include "Geometry.h"
#include "RHITexture.h"
#include "RHIDevice.h"

namespace Flux {

    using Texture = RHITexture;
    
    class AssetManager
    {
    public:
        template<typename Hash, typename T>
		using CacheMap = std::unordered_map<Hash, Ref<T>>;

        explicit AssetManager(RHIDevice& device);

        Ref<Mesh>     LoadMesh(const std::string& path);
        Ref<Texture>  LoadTexture(const std::string& path);
        Ref<Material> LoadMaterial(const std::string& path);

        void ClearCache();

    private:
        Ref<Mesh>     LoadMeshFromFile(const std::string& path);
        Ref<Texture>  LoadTextureFromFile(const std::string& path);
        Ref<Material> LoadMaterialFromFile(const std::string& path);

    private:
        RHIDevice& m_Device;

        CacheMap<std::string, Mesh>     m_MeshCache;
        CacheMap<std::string, Texture>  m_TextureCache;
        CacheMap<std::string, Material> m_MaterialCache;
    };

}