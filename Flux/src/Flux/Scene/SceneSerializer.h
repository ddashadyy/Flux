#pragma once


#include "Scene.h"

#include <filesystem>

namespace Flux {


	namespace fs = std::filesystem;

	class SceneSerializer final
	{
	public:
		SceneSerializer(const Ref<Scene>& scene);

		void Serialize(const fs::path& filepath);
		bool Deserialize(const fs::path& filepath);

	private:
		Ref<Scene> m_Scene;
	};


}