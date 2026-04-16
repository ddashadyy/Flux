#include "flpch.h"
#include "AssetManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_DISABLE_FAST_FLOAT
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <nlohmann/json.hpp>

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
	}

	Ref<Mesh> AssetManager::LoadMesh(const std::string& path)
	{
		return LoadOrGetFromCache(m_MeshCache, path, [this](const std::string& p) { return LoadMeshFromFile(p); });
	}

	Ref<Texture> AssetManager::LoadTexture(const std::string& path)
	{
		return LoadOrGetFromCache(m_TextureCache, path, [this](const std::string& p) { return LoadTextureFromFile(p); });
	}

	Ref<Material> AssetManager::LoadMaterial(const std::string& path)
	{
		return LoadOrGetFromCache(m_MaterialCache, path, [this](const std::string& p) { return LoadMaterialFromFile(p); });
	}

	void AssetManager::ClearCache()
	{
		m_MeshCache.clear();
		m_TextureCache.clear();
		m_MaterialCache.clear();
	}

	Ref<Mesh> AssetManager::LoadMeshFromFile(const std::string& path)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		FL_CORE_ASSERT(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()), err);

		std::vector<Vertex>   vertices;
		std::vector<uint32_t> indices;

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};
				vertex.Position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};
				vertex.Normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
				vertex.TexCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1]
				};

				indices.emplace_back(static_cast<uint32_t>(vertices.size()));
				vertices.emplace_back(vertex);
			}
		}

		MeshData meshData{};
		meshData.Type = vertices.size() <= 65535 ? IndexType::Uint16 : IndexType::Uint32;
		meshData.Vertices = std::move(vertices);
		meshData.Indices = std::move(indices);

		FL_CORE_INFO("Loaded mesh: {0} | Vertices: {1} | Indices: {2}", path, meshData.Vertices.size(), meshData.Indices.size());

		return CreateRef<Mesh>(m_Device, meshData);
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

	Ref<Material> AssetManager::LoadMaterialFromFile(const std::string& path)
	{
		std::ifstream file(path);
		FL_CORE_ASSERT(file.is_open(), "Failed to open material file: {0}", path);

		nlohmann::json json;
		file >> json;

		MaterialData matData{};
		matData.VertexShaderSPIRV = LoadSPIRV(json["vertexShader"].get<std::string>());
		matData.FragmentShaderSPIRV = LoadSPIRV(json["fragmentShader"].get<std::string>());

		auto material = CreateRef<Material>(m_Device, matData);

		// Текстуры (опциональные)
		if (json.contains("albedo"))
			material->SetTexture(1, LoadTexture(json["albedo"].get<std::string>()));

		if (json.contains("normal"))
			material->SetTexture(2, LoadTexture(json["normal"].get<std::string>()));

		if (json.contains("roughnessMetallic"))
			material->SetTexture(3, LoadTexture(json["roughnessMetallic"].get<std::string>()));

		// Параметры (опциональные)
		MaterialParams params{};
		if (json.contains("baseColor"))
		{
			auto c = json["baseColor"];
			params.BaseColor = { c[0], c[1], c[2], c[3] };
		}
		if (json.contains("roughness"))
			params.Properties.x = json["roughness"].get<float>();
		if (json.contains("metallic"))
			params.Properties.y = json["metallic"].get<float>();

		material->SetParams(params);

		FL_CORE_INFO("Loaded material: {0}", path);
		return material;
	}


}