#include "flpch.h"
#include "AssetManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_DISABLE_FAST_FLOAT
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <nlohmann/json.hpp>

#include <filesystem>



namespace {
	struct VertexHasher
	{
		size_t operator () (const Flux::Vertex& vertex) const
		{
			size_t seed = 0;

			auto hashCombine = [&seed](float val) {
				seed ^= std::hash<float>{}(val)+0x9e3779b9 + (seed << 6) + (seed >> 2);
			};

			hashCombine(vertex.Position.x);
			hashCombine(vertex.Position.y);
			hashCombine(vertex.Position.z);

			hashCombine(vertex.Normal.x);
			hashCombine(vertex.Normal.y);
			hashCombine(vertex.Normal.z);

			hashCombine(vertex.TexCoord.x);
			hashCombine(vertex.TexCoord.y);

			hashCombine(vertex.Tangent.x);
			hashCombine(vertex.Tangent.y);
			hashCombine(vertex.Tangent.z);

			return seed;
		}
	};
}

namespace Flux {
	
	static std::vector<uint32_t> LoadSPIRV(const std::string& path)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		FL_CORE_ASSERT(file.is_open(), "Failed to open SPIRV file: {0}", path);

		size_t fileSize = file.tellg();
		std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

		file.seekg(0);
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

		return buffer;
	}

	template <typename T, typename Loader>
	static Ref<T> LoadOrGetFromCache(AssetManager::CacheMap<std::string, T>& cache, const std::string& path, Loader loader)
	{
		auto it = cache.find(path);
		if (it != cache.end())
			return it->second;

		auto asset = loader(path);
		cache[path] = asset;
		return asset;
	}


	AssetManager::AssetManager(RHIDevice& device)
		: m_Device(device)
	{
		TextureSpec spec{};
		spec.Width = 1;
		spec.Height = 1;
		spec.ImageFormat = Format::R8G8B8A8_UNORM;
		spec.Usage = TextureUsage::Sampled;
		spec.Type = TextureType::Texture2D;

		uint8_t whitePixel[4] = { 255, 255, 255, 255 };
		m_DefaultWhiteTexture = m_Device.CreateTexture(spec);
		m_DefaultWhiteTexture->SetData(whitePixel, sizeof(whitePixel));

		uint8_t normalPixel[4] = { 128, 128, 255, 255 };
		m_DefaultNormalTexture = m_Device.CreateTexture(spec);
		m_DefaultNormalTexture->SetData(normalPixel, sizeof(normalPixel));
	}

	Ref<Model> AssetManager::LoadModel(const std::string& path, RHIDescriptorSetLayout* textureLayout)
	{
		auto it = m_ModelCache.find(path);
		if (it != m_ModelCache.end())
			return it->second;

		auto asset = LoadModelFromFile(path, textureLayout);
		m_ModelCache[path] = asset;
		return asset;
	}

	Ref<Texture> AssetManager::LoadTexture(const std::string& path)
	{
		return LoadOrGetFromCache(m_TextureCache, path, [this](const std::string& p) { return LoadTextureFromFile(p); });
	}


	void AssetManager::ClearCache()
	{
		m_ModelCache.clear();
		m_TextureCache.clear();
		
	}

	Ref<Model> AssetManager::LoadModelFromFile(const std::string& path, RHIDescriptorSetLayout* textureLayout)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		std::filesystem::path basePath = std::filesystem::path(path).parent_path();
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), basePath.string().c_str());

		if (!ret) {
			FL_CORE_ERROR("Failed to load OBJ: {0}\nError: {1}", path, err);
			return nullptr;
		}

		std::vector<Vertex> vertices;
		std::unordered_map<Vertex, uint32_t, VertexHasher> uniqueVertices;
		std::map<int, std::vector<uint32_t>> materialIndicesMap;

		for (const auto& shape : shapes)
		{
			size_t faceIndex = 0;
			for (size_t i = 0; i < shape.mesh.indices.size(); i++)
			{
				const auto& index = shape.mesh.indices[i];

				int materialId = -1;
				if (faceIndex < shape.mesh.material_ids.size())
					materialId = shape.mesh.material_ids[faceIndex];

				Vertex vertex{};
				if (index.vertex_index >= 0)
					vertex.Position = { attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2] };
				if (index.normal_index >= 0)
					vertex.Normal = { attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2] };
				else
					vertex.Normal = { 0.f, 1.f, 0.f };
				if (index.texcoord_index >= 0)
					vertex.TexCoord = { attrib.texcoords[2 * index.texcoord_index + 0], 1.0f - attrib.texcoords[2 * index.texcoord_index + 1] };
				else
					vertex.TexCoord = { 0.f, 0.f };

				if (uniqueVertices.count(vertex) == 0) 
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.emplace_back(vertex);
				}

				materialIndicesMap[materialId].emplace_back(uniqueVertices[vertex]);

				if ((i + 1) % 3 == 0)
					faceIndex++;
			}
		}

		// =============================================
		//  Расчёт тангентов
		// =============================================
		for (const auto& [matId, indices] : materialIndicesMap)
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

		// Нормализация и ортогонализация Грама-Шмидта
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

		auto model = CreateRef<Model>();

		// =============================================
		//  Создание VBO через Staging
		// =============================================
		BufferSpec vboStagingSpec{};
		vboStagingSpec.Size = vertices.size() * sizeof(Vertex);
		vboStagingSpec.Usage = Flux::BufferUsage::Staging;
		vboStagingSpec.CpuVisible = true;
		auto stagingVBO = m_Device.CreateBuffer(vboStagingSpec);
		stagingVBO->SetData(vertices.data(), vboStagingSpec.Size); 

		BufferSpec vboSpec{};
		vboSpec.Size = vboStagingSpec.Size;
		vboSpec.Usage = Flux::BufferUsage::Vertex;
		vboSpec.CpuVisible = false; 
		model->VertexBuffer = m_Device.CreateBuffer(vboSpec);

		m_Device.CopyBuffer(stagingVBO.get(), model->VertexBuffer.get()); // CPU -> GPU

		for (const auto& [matId, indices] : materialIndicesMap)
		{
			SubMesh subMesh;
			subMesh.IndexCount = static_cast<uint32_t>(indices.size());

			// =============================================
			//  Создание IBO через Staging
			// =============================================
			BufferSpec iboStagingSpec{};
			iboStagingSpec.Size = indices.size() * sizeof(uint32_t);
			iboStagingSpec.Usage = Flux::BufferUsage::Staging;
			iboStagingSpec.CpuVisible = true;
			auto stagingIBO = m_Device.CreateBuffer(iboStagingSpec);
			stagingIBO->SetData(indices.data(), iboStagingSpec.Size);

			BufferSpec iboSpec{};
			iboSpec.Size = iboStagingSpec.Size;
			iboSpec.Usage = Flux::BufferUsage::Index;
			iboSpec.CpuVisible = false; 
			subMesh.IndexBuffer = m_Device.CreateBuffer(iboSpec);

			m_Device.CopyBuffer(stagingIBO.get(), subMesh.IndexBuffer.get()); // CPU -> GPU

			subMesh.DescriptorSet = m_Device.CreateDescriptorSet(textureLayout);

			Ref<Texture> albedo, normal, roughnessMetallic;

			if (matId >= 0 && matId < (int)materials.size())
			{
				const auto& mat = materials[matId];

				if (!mat.diffuse_texname.empty())
					albedo = LoadTexture((basePath / mat.diffuse_texname).string());

				std::string normalName;
				auto it = mat.unknown_parameter.find("norm");
				if (it != mat.unknown_parameter.end())
					normalName = it->second;
				else if (!mat.normal_texname.empty())
					normalName = mat.normal_texname;
				else if (!mat.bump_texname.empty())
					normalName = mat.bump_texname;

				if (!normalName.empty())
					normal = LoadTexture((basePath / normalName).string());

				if (!mat.roughness_texname.empty())
					roughnessMetallic = LoadTexture((basePath / mat.roughness_texname).string());
			}

			subMesh.DescriptorSet->BindTexture(0, albedo ? albedo.get() : m_DefaultWhiteTexture.get());
			subMesh.DescriptorSet->BindTexture(1, normal ? normal.get() : m_DefaultNormalTexture.get());
			subMesh.DescriptorSet->BindTexture(2, roughnessMetallic ? roughnessMetallic.get() : m_DefaultWhiteTexture.get());
			subMesh.DescriptorSet->Update();

			model->Meshes.push_back(std::move(subMesh));
		}

		FL_CORE_INFO("Loaded Model: {0} | Vertices: {1} | SubMeshes(Materials): {2}", path, vertices.size(), model->Meshes.size());
		return model;
	}

	Ref<Texture> AssetManager::LoadTextureFromFile(const std::string& path)
	{
		int width = 0, height = 0, channels = 0;
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 4);

		FL_CORE_ASSERT(data, "Failed to load texture image! Failure reason is {0}", stbi_failure_reason());

		TextureSpec spec{};
		spec.Width = width;
		spec.Height = height;
		spec.Depth = 1;
		spec.MipLevels = 1;
		spec.ImageFormat = Flux::Format::R8G8B8A8_UNORM;
		spec.Usage = Flux::TextureUsage::Sampled;

		auto texture = m_Device.CreateTexture(spec);
		texture->SetData(data, width * height * 4);
		stbi_image_free(data);

		FL_CORE_INFO("Loaded texture: {0} | {1}x{2} | Channels: 4", path, width, height);

		return texture;
	}
}