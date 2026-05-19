#include "flpch.h"
#include "AssetManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#include <nlohmann/json.hpp>
#include <tiny_gltf.h>

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

namespace tinygltf {
    bool LoadImageData(Image* image, const int imageIndex, std::string* err,
        std::string* warn, int reqWidth, int reqHeight,
        const unsigned char* bytes, int size, void* userData)
    {
        int w, h, comp;
        unsigned char* data = stbi_load_from_memory(bytes, size, &w, &h, &comp, 4);
        if (!data) { if (err) *err = stbi_failure_reason(); return false; }
        image->width = w;
        image->height = h;
        image->component = 4;
        image->bits = 8;
        image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
        image->image.resize(w * h * 4);
        memcpy(image->image.data(), data, w * h * 4);
        stbi_image_free(data);
        return true;
    }

    bool WriteImageData(const std::string*, const std::string*,
        const Image*, bool,
        const FsCallbacks*, const URICallbacks*,
        std::string*, void*)
    {
        return false;
    }
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
            spec.Width = 1;
            spec.Height = 1;
            spec.Depth = 1;
            spec.MipLevels = 1;
            spec.GenerateMipmaps = false;
            spec.ImageFormat = Format::R8G8B8A8_UNORM;
            spec.Usage = TextureUsage::Sampled | TextureUsage::TransferDst;
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
    Ref<Model> AssetManager::LoadModel(const std::filesystem::path& path)
    {
        return LoadOrGetFromCache(m_ModelCache, path, [this](const auto& p) {
            return LoadModelFromFile(p, m_TextureLayout);
            });
    }

    Ref<Texture> AssetManager::LoadTexture(const std::filesystem::path& path)
    {
        return LoadOrGetFromCache(m_TextureCache, path, [this](const auto& p) {
            return LoadTextureFromFile(p);
            });
    }

    void AssetManager::InitRendererResources(RHIDescriptorSetLayout* textureLayout)
    {
        m_TextureLayout = textureLayout;
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
            return result;
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
                if (index.vertex_index >= 0)
                    vertex.Position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.Normal = (index.normal_index >= 0)
                    ? glm::vec3{
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2] }
                : glm::vec3{ 0.f, 1.f, 0.f };

                vertex.TexCoord = (index.texcoord_index >= 0)
                    ? glm::vec2{
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1] }
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

        std::string normalName = objMat.normal_texname;
        if (normalName.empty()) normalName = objMat.bump_texname;
        if (normalName.empty()) {
            auto it = objMat.unknown_parameter.find("norm");
            if (it != objMat.unknown_parameter.end()) normalName = it->second;
        }
        mat.Normal = TryLoad(normalName);

        mat.RoughnessMetallic = TryLoad(objMat.roughness_texname);

        mat.Albedo = mat.Albedo ? mat.Albedo : m_DefaultWhiteTexture;
        mat.Normal = mat.Normal ? mat.Normal : m_DefaultNormalTexture;
        mat.RoughnessMetallic = mat.RoughnessMetallic ? mat.RoughnessMetallic : m_DefaultWhiteTexture;
    }

    Ref<Model> AssetManager::LoadGltfFromFile(const std::filesystem::path& path, RHIDescriptorSetLayout* textureLayout)
    {
        tinygltf::Model gltfModel;
        tinygltf::TinyGLTF loader;
        std::string err, warn;

        bool ok = path.extension() == ".glb"
            ? loader.LoadBinaryFromFile(&gltfModel, &err, &warn, path.string())
            : loader.LoadASCIIFromFile(&gltfModel, &err, &warn, path.string());

        if (!warn.empty()) FL_CORE_WARN("glTF: {0}", warn);
        if (!ok) { FL_CORE_ERROR("Failed to load glTF: {0} | {1}", path.string(), err); return nullptr; }

        auto model = CreateRef<Model>();
        model->Path = path;

        std::vector<Vertex> allVertices;

        for (const auto& mesh : gltfModel.meshes)
        {
            for (const auto& primitive : mesh.primitives)
            {
                if (primitive.mode != TINYGLTF_MODE_TRIANGLES) continue;

                std::vector<Vertex> vertices;
                std::vector<uint32_t> indices;

                // Позиции
                {
                    const auto& acc = gltfModel.accessors[primitive.attributes.at("POSITION")];
                    const auto& bv = gltfModel.bufferViews[acc.bufferView];
                    const auto& buf = gltfModel.buffers[bv.buffer];

                    size_t stride = bv.byteStride != 0 ? bv.byteStride : sizeof(float) * 3;
                    const uint8_t* base = buf.data.data() + bv.byteOffset + acc.byteOffset;

                    vertices.resize(acc.count);
                    for (size_t i = 0; i < acc.count; i++)
                    {
                        const float* d = reinterpret_cast<const float*>(base + i * stride);
                        vertices[i].Position = { d[0], d[1], d[2] };
                    }
                }

                // Нормали
                if (primitive.attributes.count("NORMAL"))
                {
                    const auto& acc = gltfModel.accessors[primitive.attributes.at("NORMAL")];
                    const auto& bv = gltfModel.bufferViews[acc.bufferView];
                    const auto& buf = gltfModel.buffers[bv.buffer];

                    size_t stride = bv.byteStride != 0 ? bv.byteStride : sizeof(float) * 3;
                    const uint8_t* base = buf.data.data() + bv.byteOffset + acc.byteOffset;

                    for (size_t i = 0; i < acc.count; i++)
                    {
                        const float* d = reinterpret_cast<const float*>(base + i * stride);
                        vertices[i].Normal = { d[0], d[1], d[2] };
                    }
                }

                // UV
                if (primitive.attributes.count("TEXCOORD_0"))
                {
                    const auto& acc = gltfModel.accessors[primitive.attributes.at("TEXCOORD_0")];
                    const auto& bv = gltfModel.bufferViews[acc.bufferView];
                    const auto& buf = gltfModel.buffers[bv.buffer];

                    size_t stride = bv.byteStride != 0 ? bv.byteStride : sizeof(float) * 2;
                    const uint8_t* base = buf.data.data() + bv.byteOffset + acc.byteOffset;

                    for (size_t i = 0; i < acc.count; i++)
                    {
                        const float* d = reinterpret_cast<const float*>(base + i * stride);
                        vertices[i].TexCoord = { d[0], d[1] };
                    }
                }

                // ── Тангенты (vec4 — w = знак битангента) ────────────────────
                bool hasTangents = primitive.attributes.count("TANGENT") > 0;
                if (hasTangents)
                {
                    const auto& acc = gltfModel.accessors[primitive.attributes.at("TANGENT")];
                    const auto& bv = gltfModel.bufferViews[acc.bufferView];
                    const auto& buf = gltfModel.buffers[bv.buffer];

                    size_t stride = bv.byteStride != 0 ? bv.byteStride : sizeof(float) * 4;
                    const uint8_t* base = buf.data.data() + bv.byteOffset + acc.byteOffset;

                    for (size_t i = 0; i < acc.count; i++)
                    {
                        const float* d = reinterpret_cast<const float*>(base + i * stride);
                        float w = d[3];

                        glm::vec3 T = glm::normalize(glm::vec3(d[0], d[1], d[2]));
                        glm::vec3 N = glm::normalize(vertices[i].Normal);

                        T = glm::normalize(T - glm::dot(T, N) * N);

                        glm::vec3 B = glm::cross(N, T) * w;

                        if (glm::dot(glm::cross(N, T), B) < 0.0f)
                            T = T * -1.0f;

                        vertices[i].Tangent = T;
                    }
                }

                // Индексы
                {
                    const auto& acc = gltfModel.accessors[primitive.indices];
                    const auto& bv = gltfModel.bufferViews[acc.bufferView];
                    const auto& buf = gltfModel.buffers[bv.buffer];
                    const uint8_t* raw = buf.data.data() + bv.byteOffset + acc.byteOffset;

                    indices.resize(acc.count);
                    switch (acc.componentType)
                    {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                    {
                        for (size_t i = 0; i < acc.count; i++)
                            indices[i] = raw[i];
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    {
                        const uint16_t* src = reinterpret_cast<const uint16_t*>(raw);
                        for (size_t i = 0; i < acc.count; i++)
                            indices[i] = src[i];
                        break;
                    }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                    {
                        const uint32_t* src = reinterpret_cast<const uint32_t*>(raw);
                        for (size_t i = 0; i < acc.count; i++)
                            indices[i] = src[i];
                        break;
                    }
                    default:
                        FL_CORE_ERROR("glTF: unsupported index component type: {0}", acc.componentType);
                        break;
                    }
                }

                // AABB 
                for (const auto& v : vertices)
                {
                    model->Bounds.Min = glm::min(model->Bounds.Min, v.Position);
                    model->Bounds.Max = glm::max(model->Bounds.Max, v.Position);
                }

                // Вычисляем тангенты если нет в файле
                if (!hasTangents)
                {
                    std::map<int, std::vector<uint32_t>> tmp = { { 0, indices } };
                    CalculateTangents(vertices, tmp);
                }

                // Объединяем в общий vertex buffer
                uint32_t vertexOffset = (uint32_t)allVertices.size();
                for (auto& idx : indices) idx += vertexOffset;
                allVertices.insert(allVertices.end(), vertices.begin(), vertices.end());

                // Index buffer
                SubMesh subMesh{};
                subMesh.IndexCount = (uint32_t)indices.size();

                uint64_t ibSize = indices.size() * sizeof(uint32_t);
                BufferSpec ibStage{}; ibStage.Size = ibSize;
                ibStage.Usage = BufferUsage::Staging; ibStage.CpuVisible = true;
                auto ibStaging = m_Device.CreateBuffer(ibStage);
                ibStaging->SetData(indices.data(), ibSize);

                BufferSpec ibSpec{}; ibSpec.Size = ibSize;
                ibSpec.Usage = BufferUsage::Index; ibSpec.CpuVisible = false;
                subMesh.IndexBuffer = m_Device.CreateBuffer(ibSpec);
                m_Device.CopyBuffer(ibStaging.get(), subMesh.IndexBuffer.get(), ibSize);

                // Материал
                subMesh.Mat.Albedo = m_DefaultWhiteTexture;
                subMesh.Mat.Normal = m_DefaultNormalTexture;
                subMesh.Mat.RoughnessMetallic = m_DefaultWhiteTexture;
                subMesh.Mat.RoughnessOverride = -1.0f;
                subMesh.Mat.MetallicOverride = -1.0f;

                if (primitive.material >= 0)
                {
                    const auto& mat = gltfModel.materials[primitive.material];
                    const auto& pbr = mat.pbrMetallicRoughness;

                    // Albedo
                    if (pbr.baseColorTexture.index >= 0)
                        subMesh.Mat.Albedo = LoadGltfTexture(gltfModel, pbr.baseColorTexture.index);

                    // Normal map
                    if (mat.normalTexture.index >= 0)
                        subMesh.Mat.Normal = LoadGltfTexture(gltfModel, mat.normalTexture.index);

                    // Metallic-Roughness
                    if (pbr.metallicRoughnessTexture.index >= 0)
                    {
                        subMesh.Mat.RoughnessMetallic = LoadGltfTexture(gltfModel, pbr.metallicRoughnessTexture.index);
                        subMesh.Mat.RoughnessOverride = -1.0f;
                        subMesh.Mat.MetallicOverride = -1.0f;
                    }
                    else
                    {
                        subMesh.Mat.RoughnessOverride = (float)pbr.roughnessFactor;
                        subMesh.Mat.MetallicOverride = (float)pbr.metallicFactor;
                    }

                    const auto& c = pbr.baseColorFactor;
                    subMesh.Mat.Color = { (float)c[0], (float)c[1], (float)c[2], (float)c[3] };
                }

                // Descriptor set
                subMesh.Mat.DescriptorSet = m_Device.CreateDescriptorSet(textureLayout);
                subMesh.Mat.DescriptorSet->BindTexture(ALBEDO_SLOT, subMesh.Mat.Albedo.get(), m_DefaultSampler.get());
                subMesh.Mat.DescriptorSet->BindTexture(NORMAL_SLOT, subMesh.Mat.Normal.get(), m_DefaultSampler.get());
                subMesh.Mat.DescriptorSet->BindTexture(ROUGH_METAL_SLOT, subMesh.Mat.RoughnessMetallic.get(), m_DefaultSampler.get());
                subMesh.Mat.DescriptorSet->Update();

                model->Meshes.push_back(std::move(subMesh));
            }
        }

        uint64_t vbSize = allVertices.size() * sizeof(Vertex);
        BufferSpec vbStage{}; vbStage.Size = vbSize;
        vbStage.Usage = BufferUsage::Staging; vbStage.CpuVisible = true;
        auto vbStaging = m_Device.CreateBuffer(vbStage);
        vbStaging->SetData(allVertices.data(), vbSize);

        BufferSpec vbSpec{}; vbSpec.Size = vbSize;
        vbSpec.Usage = BufferUsage::Vertex; vbSpec.CpuVisible = false;
        model->VertexBuffer = m_Device.CreateBuffer(vbSpec);
        m_Device.CopyBuffer(vbStaging.get(), model->VertexBuffer.get(), vbSize);

        FL_CORE_INFO("Loaded glTF: {0} | Vertices: {1} | SubMeshes: {2}",
            path.string(), allVertices.size(), model->Meshes.size());

        return model;
    }

    Ref<Texture> AssetManager::LoadGltfTexture(const tinygltf::Model& gltfModel, int textureIndex)
    {
        const auto& tex = gltfModel.textures[textureIndex];
        const auto& image = gltfModel.images[tex.source];

        if (!image.image.empty())
        {
            FL_CORE_ASSERT(image.component == 4 || image.component == 3,
                "glTF texture: unexpected component count {0}", image.component);

            std::vector<uint8_t> rgba;
            const uint8_t* pixels = image.image.data();

            if (image.component == 3)
            {
                rgba.resize(image.width * image.height * 4);
                for (int i = 0; i < image.width * image.height; i++)
                {
                    rgba[i * 4 + 0] = image.image[i * 3 + 0];
                    rgba[i * 4 + 1] = image.image[i * 3 + 1];
                    rgba[i * 4 + 2] = image.image[i * 3 + 2];
                    rgba[i * 4 + 3] = 255;
                }
                pixels = rgba.data();
            }

            TextureSpec spec{};
            spec.Width = image.width;
            spec.Height = image.height;
            spec.Depth = 1;
            spec.MipLevels = 1;
            spec.GenerateMipmaps = true;
            spec.ImageFormat = Format::R8G8B8A8_UNORM;
            spec.Usage = TextureUsage::Sampled;

            auto texture = m_Device.CreateTexture(spec);
            texture->SetData(pixels, (size_t)image.width * image.height * 4);
            return texture;
        }

        if (!image.uri.empty())
            return LoadTexture(image.uri);

        FL_CORE_WARN("glTF texture {0}: no data, using default", textureIndex);
        return m_DefaultWhiteTexture;
    }

    // ==========================================
    //  Главная функция сборки модели
    // ==========================================
    Ref<Model> AssetManager::LoadModelFromFile(const std::filesystem::path& path, RHIDescriptorSetLayout* textureLayout)
    {
        std::string ext = path.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".gltf" || ext == ".glb")
            return LoadGltfFromFile(path, textureLayout);

        // --- OBJ ---
        auto model = CreateRef<Model>();
        model->Path = path;

        ParsedMeshData parsedData = ParseObj(path);
        if (parsedData.Vertices.empty()) return nullptr;

        CalculateTangents(parsedData.Vertices, parsedData.MaterialIndicesMap);

        for (const auto& v : parsedData.Vertices)
        {
            model->Bounds.Min = glm::min(model->Bounds.Min, v.Position);
            model->Bounds.Max = glm::max(model->Bounds.Max, v.Position);
        }

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

        for (const auto& [matId, indices] : parsedData.MaterialIndicesMap)
        {
            SubMesh subMesh{};
            subMesh.IndexCount = static_cast<uint32_t>(indices.size());

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

            if (matId >= 0 && matId < (int)parsedData.Materials.size())
                LoadMaterialTextures(subMesh.Mat, parsedData.Materials[matId], parsedData.BasePath);
            else
                LoadMaterialTextures(subMesh.Mat, {}, parsedData.BasePath);

            subMesh.Mat.DescriptorSet = m_Device.CreateDescriptorSet(textureLayout);
            subMesh.Mat.DescriptorSet->BindTexture(ALBEDO_SLOT, subMesh.Mat.Albedo.get(), m_DefaultSampler.get());
            subMesh.Mat.DescriptorSet->BindTexture(NORMAL_SLOT, subMesh.Mat.Normal.get(), m_DefaultSampler.get());
            subMesh.Mat.DescriptorSet->BindTexture(ROUGH_METAL_SLOT, subMesh.Mat.RoughnessMetallic.get(), m_DefaultSampler.get());
            subMesh.Mat.DescriptorSet->Update();

            model->Meshes.push_back(std::move(subMesh));
        }

        FL_CORE_INFO("Loaded Model: {0} | Vertices: {1} | SubMeshes: {2}",
            path.string(), parsedData.Vertices.size(), model->Meshes.size());
        return model;
    }


    // ==========================================
    //  Загрузка текстуры
    // ==========================================
    Ref<Texture> AssetManager::LoadTextureFromFile(const std::filesystem::path& path)
    {
        int width = 0, height = 0, channels = 0;
        stbi_uc* data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);

        FL_CORE_ASSERT(data, "Failed to load texture: {0} | Reason: {1}",
            path.string(), stbi_failure_reason());

        TextureSpec spec{};
        spec.Width = width;
        spec.Height = height;
        spec.Depth = 1;
        spec.MipLevels = 1;
        spec.GenerateMipmaps = true;
        spec.ImageFormat = Format::R8G8B8A8_UNORM;
        spec.Usage = TextureUsage::Sampled;

        auto texture = m_Device.CreateTexture(spec);
        texture->SetData(data, width * height * 4);
        stbi_image_free(data);

        FL_CORE_INFO("Loaded texture: {0} [{1}x{2}]", path.string(), width, height);
        return texture;
    }
}