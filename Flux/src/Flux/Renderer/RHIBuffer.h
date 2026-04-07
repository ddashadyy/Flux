#pragma once


#include <Flux/Core.h>

namespace Flux {

	enum class IndexType : uint8_t
	{
		None   = 0,
		Uint16 = 1,
		Uint32 = 2
	};

	enum class BufferUsage : uint8_t
	{
		Vertex  = 1 << 0,
		Index   = 1 << 1,
		Uniform = 1 << 2,
		Storage = 1 << 3,
		Staging = 1 << 4
	};

	struct BufferSpec
	{
		uint64_t      Size       = 0;
		BufferUsage   Usage      = BufferUsage::Vertex;
		bool          CpuVisible = false;
	};


	class RHIBuffer
	{
	public:
		virtual ~RHIBuffer() = default;

		virtual void* Map()   = 0;
		virtual void  Unmap() = 0;

		virtual uint64_t    GetSize()  = 0;
		virtual BufferUsage GetUsage() = 0;

		void SetData(const void* data, uint64_t size)
		{
			void* ptr = Map();
			memcpy(ptr, data, size);
			Unmap();
		}
	};

}