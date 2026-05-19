#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define  GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <string>
#include <vector>

#include "Flux/Renderer/AssetManager.h" 
#include "Flux/Core/UUID.h"


namespace Flux {

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(UUID id) : ID(id) {}
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : Tag(tag) {}
	};

	struct TransformComponent
	{
		glm::vec3 Translation = { 0, 0, 0 };
		glm::quat Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 Scale = { 1, 1, 1 };

		glm::mat4 WorldMatrix = glm::mat4(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;

		glm::mat4 GetLocalMatrix() const
		{
			glm::mat4 t = glm::translate(glm::mat4(1.0f), Translation);
			glm::mat4 r = glm::toMat4(Rotation);
			glm::mat4 s = glm::scale(glm::mat4(1.0f), Scale);
			return t * r * s;
		}

		glm::vec3 GetEulerAngles() const { return glm::degrees(glm::eulerAngles(Rotation)); }
		void SetEulerAngles(const glm::vec3& eulerDegrees) { Rotation = glm::quat(glm::radians(eulerDegrees)); }
	};

	struct MeshComponent
	{
		Ref<Model> Model = nullptr;
	};

	struct LightComponent
	{
		enum class LightType { Point, Directional, Spot } Type = LightType::Point;
		glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
		float Intensity = 1.0f;
		float Range = 10.0f;
	};

	struct RelationshipComponent
	{
		UUID ParentID = nullptr; // нет родителя
		std::vector<UUID> Children;
	};

	struct DestroyFlag {}; // тег, помечающий сущность на удаление

}