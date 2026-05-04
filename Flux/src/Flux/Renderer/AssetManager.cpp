#include "flpch.h"
#include "AssetManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


namespace {
    struct VertexHasher
    {
        size_t operator()(const Flux::Vertex& vertex) const
        {
            size_t seed = 0;
            auto hashCombine = [&seed](float val) {
                seed ^= std::hash<float>{}(val)+0x9e3779b9 + (seed << 6) + (seed >> 2);
                };

            hashCombine(vertex.Position.x); hashCombine(vertex.Position.y); hashCombine(vertex.Position.z);
            hashCombine(vertex.Normal.x);   hashCombine(vertex.Normal.y);   hashCombine(vertex.Normal.z);
            hashCombine(vertex.TexCoord.x); hashCombine(vertex.TexCoord.y);
            hashCombine(vertex.Tangent.x);  hashCombine(vertex.Tangent.y);  hashCombine(vertex.Tangent.z);

            return seed;
        }
    };
}

namespace Flux {

    template <typename T, typename Loader>
    static Ref<T> LoadOrGetFromCache(AssetManager::CacheMap<std::string, T>& cache, const std::filesystem::path& path, Loader loader)
    {
        std::string pathStr = path.string();
        auto it = cache.find(pathStr);
        if (it != cache.end()) return it->second;

        auto asset = loader(path);
        cache[pathStr] = asset;
        return asset;
    }

    // ==========================================
    //  Конструктор
    // ==========================================
    AssetManager::AssetManager(RHIDevice& device) : m_Device(device)
    {
        auto Create1x1Texture = [this](const uint8_t pixel[4]) -> Ref<Texture> {
            TextureSpec spec{};
            spec.Width = 1; spec.Height = 1; spec.Depth = 1;
            spec.MipLevels = 1;
            spec.ImageFormat = Format::R8G8B8A8_UNORM;
            spec.Usage = TextureUsage::Sampled;
            spec.Type = TextureType::Texture2D;

            auto tex = m_Device.CreateTexture(spec);
            tex->SetData(pixel, sizeof(uint8_t) * 4);
            return tex;
            };

        uint8_t whitePixel[4] = { 255, 255, 255, 255 };
        m_DefaultWhiteTexture = Create1x1Texture(whitePixel);

        uint8_t normalPixel[4] = { 128, 128, 255, 255 };
        m_DefaultNormalTexture = Create1x1Texture(normalPixel);

        SamplerSpec spec{};
        spec.MagFilter = FilterMode::Linear;
        spec.MinFilter = FilterMode::Linear;
        spec.MipMode = MipMapMode::Linear;
        spec.AddressU = AddressMode::Repeat;
        spec.AddressV = AddressMode::Repeat;
        spec.AnisotropyEnable = true;
        spec.MaxAnisotropy = 16.0f;
        m_DefaultSampler = m_Device.CreateSampler(spec);
    }

    // ==========================================
    //  Публичные методы загрузки
    // ==========================================
    Ref<Model> AssetManager::LoadModel(const std::filesystem::path& path, RHIDescriptorSetLayout* textureLayout)
    {
        return LoadOrGetFromCache(m_ModelCache, path, [this, textureLayout](const auto& p) {
            return LoadModelFromFile(p, textureLayout);
            });
    }

    Ref<Texture> AssetManager::LoadTexture(const std::filesystem::path& path)
    {
        return LoadOrGetFromCache(m_TextureCache, path, [this](const auto& p) {
            return LoadTextureFromFile(p);
            });
    }

    void AssetManager::ClearCache()
    {
        m_ModelCache.clear();
        m_TextureCache.clear();
    }

    // ==========================================
    //  Парсинг OBJ
    // ==========================================
    AssetManager::ParsedMeshData AssetManager::ParseObj(const std::filesystem::path& path)
    {
        ParsedMeshData result;
        result.BasePath = path.parent_path();

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &result.Materials, &warn, &err, path.string().c_str(), result.BasePath.string().c_str()))
        {
            FL_CORE_ERROR("Failed to load OBJ: {0}\nError: {1}", path.string(), err);
            return result; // Вернет пустые данные
        }

        std::unordered_map<Vertex, uint32_t, VertexHasher> uniqueVertices;

        for (const auto& shape : shapes)
        {
            size_t faceIndex = 0;
            for (size_t i = 0; i < shape.mesh.indices.size(); i++)
            {
                const auto& index = shape.mesh.indices[i];
                int materialId = (faceIndex < shape.mesh.material_ids.size()) ? shape.mesh.material_ids[faceIndex] : -1;

                Vertex vertex{};
                if (index.vertex_index >= 0) vertex.Position = { attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2] };

                vertex.Normal = (index.normal_index >= 0)
                    ? glm::vec3{ attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2] }
                : glm::vec3{ 0.f, 1.f, 0.f };

                vertex.TexCoord = (index.texcoord_index >= 0)
                    ? glm::vec2{ attrib.texcoords[2 * index.texcoord_index + 0], 1.0f - attrib.texcoords[2 * index.texcoord_index + 1] }
                : glm::vec2{ 0.f, 0.f };

                if (uniqueVertices.count(vertex) == 0)
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(result.Vertices.size());
                    result.Vertices.emplace_back(vertex);
                }

                result.MaterialIndicesMap[materialId].emplace_back(uniqueVertices[vertex]);

                if ((i + 1) % 3 == 0) faceIndex++;
            }
        }
        return result;
    }

    // ==========================================
    //  Расчет тангентов
    // ==========================================
    void AssetManager::CalculateTangents(std::vector<Vertex>& vertices, const std::map<int, std::vector<uint32_t>>& indicesMap)
    {
        for (const auto& [matId, indices] : indicesMap)
        {
            for (size_t i = 0; i + 2 < indices.size(); i += 3)
            {
                Vertex& v0 = vertices[indices[i]];
                Vertex& v1 = vertices[indices[i + 1]];
                Vertex& v2 = vertices[indices[i + 2]];

                glm::vec3 edge1 = v1.Position - v0.Position;
                glm::vec3 edge2 = v2.Position - v0.Position;
                glm::vec2 dUV1 = v1.TexCoord - v0.TexCoord;
                glm::vec2 dUV2 = v2.TexCoord - v0.TexCoord;

                float det = dUV1.x * dUV2.y - dUV2.x * dUV1.y;
                if (std::abs(det) < 1e-6f) continue;

                float inv = 1.0f / det;
                glm::vec3 tangent = inv * (dUV2.y * edge1 - dUV1.y * edge2);

                v0.Tangent += tangent;
                v1.Tangent += tangent;
                v2.Tangent += tangent;
            }
        }

        for (auto& v : vertices)
        {
            if (glm::length(v.Tangent) > 1e-6f)
                v.Tangent = glm::normalize(v.Tangent - glm::dot(v.Tangent, v.Normal) * v.Normal);
            else
            {
                glm::vec3 up = std::abs(v.Normal.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
                v.Tangent = glm::normalize(glm::cross(up, v.Normal));
            }
        }
    }

    // ==========================================
    //  Загрузка текстур материала
    // ==========================================
    void AssetManager::LoadMaterialTextures(Material& mat, const tinyobj::material_t& objMat, const std::filesystem::path& baseDir)
    {
        auto TryLoad = [&](const std::string& name) -> Ref<Texture> {
            if (name.empty()) return nullptr;
            return LoadTexture(baseDir / name);
            };

        mat.Albedo = TryLoad(objMat.diffuse_texname);

        // Логика поиска нормалей (от конкретного к общему)
        std::string normalName = objMat.normal_texname;
        if (normalName.empty()) normalName = objMat.bump_texname;
        if (normalName.empty()) {
            auto it = objMat.unknown_parameter.find("norm");
            if (it != objMat.unknown_parameter.end()) normalName = it->second;
        }
        mat.Normal = TryLoad(normalName);

        mat.RoughnessMetallic = TryLoad(objMat.roughness_texname);

        // Fallback на дефолтные текстуры (через тернарный оператор компактнее)
        mat.Albedo = mat.Albedo ? mat.Albedo : m_DefaultWhiteTexture;
        mat.Normal = mat.Normal ? mat.Normal : m_DefaultNormalTexture;
        mat.RoughnessMetallic = mat.RoughnessMetallic ? mat.RoughnessMetallic : m_DefaultWhiteTexture;
    }

    // ==========================================
    //  Главная функция сборки модели
    // ==========================================
    Ref<Model> AssetManager::LoadModelFromFile(const std::filesystem::path& path, RHIDescriptorSetLayout* textureLayout)
    {
        auto model = CreateRef<Model>();

        // 1. Парсинг
        ParsedMeshData parsedData = ParseObj(path);
        if (parsedData.Vertices.empty()) return nullptr;

        // 2. Математика
        CalculateTangents(parsedData.Vertices, parsedData.MaterialIndicesMap);

        // 3. Создание общего VBO (ИСПРАВЛЕНИЕ БАГА)
        size_t vertexBufferSize = parsedData.Vertices.size() * sizeof(Vertex);

        BufferSpec vboStagingSpec{};
        vboStagingSpec.Size = vertexBufferSize;
        vboStagingSpec.Usage = BufferUsage::Staging;
        vboStagingSpec.CpuVisible = true;
        auto vboStaging = m_Device.CreateBuffer(vboStagingSpec);
        vboStaging->SetData(parsedData.Vertices.data(), vertexBufferSize, 0);

        BufferSpec vboSpec{};
        vboSpec.Size = vertexBufferSize;
        vboSpec.Usage = BufferUsage::Vertex;
        vboSpec.CpuVisible = false;

        
        model->VertexBuffer = m_Device.CreateBuffer(vboSpec);
        m_Device.CopyBuffer(vboStaging.get(), model->VertexBuffer.get(), vertexBufferSize);


        // 4. Создание сабмэшей (IBO) и материалов
        for (const auto& [matId, indices] : parsedData.MaterialIndicesMap)
        {
            SubMesh subMesh{};
            subMesh.IndexCount = static_cast<uint32_t>(indices.size());

            // --- ИНДЕКСНЫЙ БУФЕР ---
            size_t indexBufferSize = indices.size() * sizeof(uint32_t);

            BufferSpec iboStagingSpec{};
            iboStagingSpec.Size = indexBufferSize;
            iboStagingSpec.Usage = BufferUsage::Staging;
            iboStagingSpec.CpuVisible = true;
            auto iboStaging = m_Device.CreateBuffer(iboStagingSpec);
            iboStaging->SetData(indices.data(), indexBufferSize, 0);

            BufferSpec iboSpec{};
            iboSpec.Size = indexBufferSize;
            iboSpec.Usage = BufferUsage::Index;
            iboSpec.CpuVisible = false;
            subMesh.IndexBuffer = m_Device.CreateBuffer(iboSpec);
            m_Device.CopyBuffer(iboStaging.get(), subMesh.IndexBuffer.get(), indexBufferSize, 0, 0);

            // --- МАТЕРИАЛЫ ---
            if (matId >= 0 && matId < (int)parsedData.Materials.size())
                LoadMaterialTextures(subMesh.Mat, parsedData.Materials[matId], parsedData.BasePath);
            else
                LoadMaterialTextures(subMesh.Mat, {}, parsedData.BasePath); // Дефолты, если нет материала

            // --- ДЕСКРИПТОРНЫЙ НАБОР ---
            subMesh.Mat.DescriptorSet = m_Device.CreateDescriptorSet(textureLayout);
            subMesh.Mat.DescriptorSet->BindTexture(ALBEDO_SLOT, subMesh.Mat.Albedo.get(), m_DefaultSampler.get());
            subMesh.Mat.DescriptorSet->BindTexture(NORMAL_SLOT, subMesh.Mat.Normal.get(), m_DefaultSampler.get());
            subMesh.Mat.DescriptorSet->BindTexture(ROUGH_METAL_SLOT, subMesh.Mat.RoughnessMetallic.get(), m_DefaultSampler.get());
            subMesh.Mat.DescriptorSet->Update();

            model->Meshes.push_back(std::move(subMesh));
        }

        FL_CORE_INFO("Loaded Model: {0} | Vertices: {1} | SubMeshes: {2}", path.string(), parsedData.Vertices.size(), model->Meshes.size());
        return model;
    }

    // ==========================================
    //  Загрузка текстуры
    // ==========================================
    Ref<Texture> AssetManager::LoadTextureFromFile(const std::filesystem::path& path)
    {
        int width = 0, height = 0, channels = 0;
        stbi_uc* data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);

        FL_CORE_ASSERT(data, "Failed to load texture: {0} | Reason: {1}", path.string(), stbi_failure_reason());

        TextureSpec spec{};
        spec.Width = width;
        spec.Height = height;
        spec.Depth = 1;
        spec.MipLevels = 1; 
        spec.ImageFormat = Format::R8G8B8A8_UNORM;
        spec.Usage = TextureUsage::Sampled;

        auto texture = m_Device.CreateTexture(spec);
        texture->SetData(data, width * height * 4);
        stbi_image_free(data);

        FL_CORE_INFO("Loaded texture: {0} [{1}x{2}]", path.string(), width, height);
        return texture;
    }
}