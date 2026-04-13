#pragma once


#include "RHIBuffer.h"
#include "RHITexture.h"
#include "RHICommon.h"

#include <vector>

namespace Flux {

    enum class DescriptorType : uint8_t
    {
        UniformBuffer        = 0,
        StorageBuffer        = 1,
		CombinedImageSampler = 2,
        StorageImage         = 3
	};

    struct DescriptorBinding
    {
        uint32_t        Binding = 0;
        DescriptorType  Type    = DescriptorType::UniformBuffer;
		ShaderStage 	Stage   = ShaderStage::Vertex;
		uint32_t 	    Count   = 1;

	};

    struct DescriptorSetLayoutDesc 
    {
        std::vector<DescriptorBinding> Bindings = {};
    };

    class RHIDescriptorSetLayout
    {
    public:
		virtual ~RHIDescriptorSetLayout() = default;

        template<typename T>
        T GetHandle() const { return reinterpret_cast<T>(GetHandleImpl()); }

        virtual uint32_t GetBindingCount() const = 0;

    protected: 
        virtual void* GetHandleImpl() const = 0;
    };


    class RHIDescriptorSet
    {
    public:
		virtual ~RHIDescriptorSet() = default;

        template<typename T>
        T GetHandle() const { return reinterpret_cast<T>(GetHandleImpl()); }
        
		virtual void BindBuffer(uint32_t binding, const RHIBuffer* buffer)    = 0;
		virtual void BindTexture(uint32_t binding, const RHITexture* texture) = 0;
        virtual void Update() = 0;

    protected:
        virtual void* GetHandleImpl() const = 0;
    };
}