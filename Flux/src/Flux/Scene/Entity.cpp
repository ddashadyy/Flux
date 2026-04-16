#include "flpch.h"
#include "Entity.h"

namespace Flux {

	Entity::Entity(Ref<Mesh> mesh, Ref<Material> material)
	{
		m_Mesh     = std::move(mesh);
		m_Material = std::move(material);
	}

	void Entity::Draw(RHICommandList& cmd) const
	{
		cmd.BindDescriptorSet(&m_Material->GetDescriptorSet());
		m_Mesh->Draw(cmd);
	}

}