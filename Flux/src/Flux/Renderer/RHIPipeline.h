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

	struct PipelineDesc 
	{
		const RHIShader*              VertexShader         = nullptr;
		const RHIShader*              FragmentShader       = nullptr;
		BufferLayout                  VertexLayout;
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

		virtual PipelineType GetType()    const = 0;
		virtual bool         IsValid()    const = 0;
	};

}