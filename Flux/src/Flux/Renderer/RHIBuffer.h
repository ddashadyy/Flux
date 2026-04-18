#pragma once


#include "Flux/Core/Base.h"

namespace Flux {

	enum class IndexType : uint8_t
	{
		None   = 0,
		Uint16 = 1,
		Uint32 = 2
	};

	enum class BufferUsage : uint8_t
	{
		Vertex  = BIT(0),
		Index   = BIT(1),
		Uniform = BIT(2),
		Storage = BIT(3),
		Staging = BIT(4)
	};
	ENABLE_BITWISE_OPERATORS(BufferUsage);

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

		template<typename T>
		T GetHandle() const { return reinterpret_cast<T>(GetHandleImpl()); }

		virtual void* Map()   = 0;
		virtual void  Unmap() = 0;

		virtual uint64_t    GetSize()  const = 0;
		virtual BufferUsage GetUsage() const = 0;

		void SetData(const void* data, uint64_t size)
		{
			void* ptr = Map();
			memcpy(ptr, data, size);
			Unmap();
		}

	protected:
		virtual void* GetHandleImpl() const = 0;
	};

}