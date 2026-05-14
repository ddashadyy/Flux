#pragma once

#include "Geometry.h"
#include "RHITexture.h"
#include "RHIDevice.h"

#include <filesystem>

#include <tiny_obj_loader.h>
#include <tiny_gltf.h>

namespace Flux {

    class AssetManager
    {
    public:
        template<typename Hash, typename T>
        using CacheMap = std::unordered_map<Hash, Ref<T>>;

        explicit AssetManager(RHIDevice& device);

        Ref<Model>   LoadModel(const std::filesystem::path& path, RHIDescriptorSetLayout* textureLayout);
        Ref<Texture> LoadTexture(const std::filesystem::path& path);

        void ClearCache();

    private:
        struct ParsedMeshData 
        {
            std::vector<Vertex> Vertices;
            std::map<int, std::vector<uint32_t>> MaterialIndicesMap;
            std::vector<tinyobj::material_t> Materials;
            std::filesystem::path BasePath;
        };

        Ref<Model>   LoadModelFromFile(const std::filesystem::path& path, RHIDescriptorSetLayout* textureLayout);
        Ref<Texture> LoadTextureFromFile(const std::filesystem::path& path);

        ParsedMeshData ParseObj(const std::filesystem::path& path);
        void CalculateTangents(std::vector<Vertex>& vertices, const std::map<int, std::vector<uint32_t>>& indicesMap);
        void LoadMaterialTextures(Material& mat, const tinyobj::material_t& objMat, const std::filesystem::path& baseDir);

    private:
        Ref<Model> LoadGltfFromFile(const std::filesystem::path& path, RHIDescriptorSetLayout* textureLayout);
        Ref<Texture> LoadGltfTexture(const tinygltf::Model& gltfModel, int textureIndex);

    private:
        RHIDevice& m_Device;

        CacheMap<std::string, Model>   m_ModelCache;
        CacheMap<std::string, Texture> m_TextureCache;

        Ref<Texture> m_DefaultWhiteTexture;
        Ref<Texture> m_DefaultNormalTexture;

        Scope<RHISampler> m_DefaultSampler;

        // Магические числа (соответствуют слотам в шейдере)
        static constexpr int ALBEDO_SLOT = 0;
        static constexpr int NORMAL_SLOT = 1;
        static constexpr int ROUGH_METAL_SLOT = 2;
    };

}