#include "flpch.h"
#include "SceneSerializer.h"
#include "Components.h"
#include "Entity.h"

#include "Flux/Core/Application.h"

#include <nlohmann/json.hpp>

namespace Flux {

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
	}

	void SceneSerializer::Serialize(const fs::path& filepath)
	{
		using json = nlohmann::json;

		json sceneJson;
		sceneJson["Name"] = "Untitled Scene";

		auto& registry = m_Scene->GetRegistry();
		auto view = registry.view<IDComponent, TagComponent, TransformComponent>();

		for (auto entityHandle : view)
		{
			Entity entity(entityHandle, m_Scene.get());

			json entityJson;

			// ID, name
			entityJson["ID"] = static_cast<uint64_t>(entity.GetComponent<IDComponent>().ID);
			entityJson["Name"] = entity.GetTag();

			// Transform
			auto& tf = entity.GetTransform();
			entityJson["Transform"]["Translation"] = { tf.Translation.x, tf.Translation.y, tf.Translation.z };

			glm::vec3 rot = tf.GetEulerAngles();
			entityJson["Transform"]["Rotation"] = { rot.x, rot.y, rot.z };
			entityJson["Transform"]["Scale"] = { tf.Scale.x, tf.Scale.y, tf.Scale.z };

			// Mesh
			if (entity.HasComponent<MeshComponent>())
			{
				auto& mesh = entity.GetComponent<MeshComponent>();
				if (mesh.Model)
					entityJson["Mesh"]["Path"] = mesh.Model->GetPath().string();
			}

			// Light
			if (entity.HasComponent<LightComponent>())
			{
				auto& light = entity.GetComponent<LightComponent>();
				entityJson["Light"]["Type"] = static_cast<int>(light.Type);
				entityJson["Light"]["Color"] = { light.Color.x, light.Color.y, light.Color.z };
				entityJson["Light"]["Intensity"] = light.Intensity;
				entityJson["Light"]["Range"] = light.Range;
			}

			// Relationship
			if (entity.HasComponent<RelationshipComponent>())
			{
				entityJson["Relationship"]["ParentID"] = static_cast<uint64_t>(entity.GetComponent<RelationshipComponent>().ParentID);
			}

			sceneJson["Entities"].emplace_back(entityJson);
		}

		std::ofstream outfile(filepath, std::ios::out | std::ios::binary);
		if (outfile.is_open()) outfile << sceneJson.dump(4); 
	}

    bool SceneSerializer::Deserialize(const fs::path& filepath)
    {
        if (!std::filesystem::exists(filepath)) return false;

        std::ifstream infile(filepath, std::ios::in | std::ios::binary);
        if (!infile.is_open()) return false;

        using json = nlohmann::json;

        json sceneJson;
        infile >> sceneJson;

        m_Scene->Clear();

        std::unordered_map<UUID, Entity> entityMap;

        if (!sceneJson.contains("Entities")) return true;

        auto& assetManager = Application::Get().GetAssetManager();

        for (auto& entityJson : sceneJson["Entities"])
        {
            UUID loadedUUID(entityJson.value("ID", (uint64_t)0));
            std::string name = entityJson.value("Name", std::string("Unnamed"));

            if (loadedUUID == UUID(nullptr)) continue;

            Entity entity = m_Scene->CreateEntityWithID(loadedUUID, name);

            // Transform
            if (entityJson.contains("Transform"))
            {
                auto& tf = entity.GetComponent<TransformComponent>();

                if (entityJson["Transform"].contains("Translation")) {
                    auto& pos = entityJson["Transform"]["Translation"];
                    if (pos.is_array() && pos.size() >= 3)
                        tf.Translation = { pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>() };
                }

                if (entityJson["Transform"].contains("Rotation")) {
                    auto& rot = entityJson["Transform"]["Rotation"];
                    if (rot.is_array() && rot.size() >= 3)
                        tf.SetEulerAngles({ rot[0].get<float>(), rot[1].get<float>(), rot[2].get<float>() });
                }

                if (entityJson["Transform"].contains("Scale")) {
                    auto& scl = entityJson["Transform"]["Scale"];
                    if (scl.is_array() && scl.size() >= 3)
                        tf.Scale = { scl[0].get<float>(), scl[1].get<float>(), scl[2].get<float>() };
                }
            }

            // Mesh
            if (entityJson.contains("Mesh") && entityJson["Mesh"].contains("Path"))
            {
                std::string meshPath = entityJson["Mesh"]["Path"].get<std::string>();
                if (!meshPath.empty())
                {
                    auto model = assetManager.LoadModel(meshPath);
                    if (model)
                        entity.AddComponent<MeshComponent>(model);
                }
            }

            // Light
            if (entityJson.contains("Light"))
            {
                auto& light = entity.AddComponent<LightComponent>();
                light.Type = (LightComponent::LightType)entityJson["Light"].value("Type", 0);

                if (entityJson["Light"].contains("Color")) 
                {
                    auto& col = entityJson["Light"]["Color"];
                    if (col.is_array() && col.size() >= 3)
                        light.Color = { col[0].get<float>(), col[1].get<float>(), col[2].get<float>() };
                }

                light.Intensity = entityJson["Light"].value("Intensity", 1.0f);
                light.Range = entityJson["Light"].value("Range", 10.0f);
            }

            entityMap[loadedUUID] = entity;
        }

        // Relationship
        for (auto& entity_json : sceneJson["Entities"])
        {
            if (entity_json.contains("Relationship") && entity_json["Relationship"].contains("ParentID"))
            {
                UUID loadedUUID(entity_json.value("ID", (uint64_t)0));
                UUID parentID(entity_json["Relationship"]["ParentID"].get<uint64_t>());

                if (parentID != UUID(nullptr) && entityMap.find(loadedUUID) != entityMap.end() && entityMap.find(parentID) != entityMap.end())
                {
                    Entity child = entityMap[loadedUUID];
                    Entity parent = entityMap[parentID];
                    m_Scene->SetParent(child, parent);
                }
            }
        }

        return true;
    }

}