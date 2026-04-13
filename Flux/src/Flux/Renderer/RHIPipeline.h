#pragma once


#include "Flux/Core.h"

#include "RHIShader.h"
#include "RHIDescriptorSet.h"
#include "RHIRenderPass.h"
#include "BufferLayout.h"


namespace Flux {

	

	
	struct BlendState
	{
		bool Enable = false;
		// расширить позже
	};

	struct DepthStencilState 
	{
		bool DepthTest = true;
		bool DepthWrite = true;
	};

	enum class PrimitiveTopology : uint8_t 
	{
		TriangleList = 0,
		TriangleStrip = 1,
		LineList = 2,
		PointList = 3,
	};

	enum class PipelineType : uint8_t
	{
		Graphics = 0,
		Compute = 1,
	};

	struct PipelineLayoutDesc
	{
		ShaderStage Stage = ShaderStage::Vertex | ShaderStage::Fragment;
		uint32_t Offset   = 0;
		uint32_t Size     = 0;
	};

	struct PipelineDesc 
	{
		const RHIShader*              VertexShader         = nullptr;
		const RHIShader*              FragmentShader       = nullptr;
		BufferLayout                  VertexLayout;
		PipelineLayoutDesc            pipelineLayoutDesc;
		BlendState                    Blend;
		DepthStencilState             DepthStencil;
		PrimitiveTopology             Topology             = PrimitiveTopology::TriangleList;
		PipelineType                  Type                 = PipelineType::Graphics;
		const RHIDescriptorSetLayout* DescriptorSetLayout  = nullptr;
		const RHIRenderPass*          RenderPass           = nullptr;
	};


	class RHIPipeline 
	{
	public:
		virtual ~RHIPipeline() = default;

		template<typename T>
		T GetHandle() const { return reinterpret_cast<T>(GetHandleImpl()); }

		template<typename T>
		T GetLayout() const { return reinterpret_cast<T>(GetLayoutImpl()); }

		virtual PipelineType GetType()    const = 0;
		virtual bool         IsValid()    const = 0;

		virtual const PipelineLayoutDesc& GetLayoutDesc() const = 0;

	protected:
		virtual void* GetHandleImpl() const = 0;
		virtual void* GetLayoutImpl() const = 0;
	};

}