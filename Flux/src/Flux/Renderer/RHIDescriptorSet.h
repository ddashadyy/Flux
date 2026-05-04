#pragma once

#include "RHICommon.h"

#include <vector>

namespace Flux {

    class RHIBuffer;
    class RHITexture;
    class RHISampler;

    // -------------------------------------------------------------------------
    // Descriptor types & layout
    // -------------------------------------------------------------------------

    enum class DescriptorType : uint8_t
    {
        UniformBuffer        = 0,
        StorageBuffer        = 1,
        CombinedImageSampler = 2,  // texture + sampler вместе (Vulkan-style)
        SampledImage         = 3,  // только texture (D3D12 / Metal style — SRV)
        Sampler              = 4,  // только sampler (D3D12 / Metal style)
        StorageImage         = 5,  // read/write texture в compute
    };

    struct DescriptorBinding
    {
        uint32_t       Binding = 0;
        DescriptorType Type    = DescriptorType::UniformBuffer;
        ShaderStage    Stage   = ShaderStage::Vertex;
        uint32_t       Count   = 1; // для массивов текстур
    };

    struct DescriptorSetLayoutDesc
    {
        std::vector<DescriptorBinding> Bindings = {};
        const char* DebugName = nullptr;
    };

    // -------------------------------------------------------------------------
    // Descriptor set layout
    // -------------------------------------------------------------------------

    class RHIDescriptorSetLayout
    {
    public:
        virtual ~RHIDescriptorSetLayout() = default;

        template<typename T>
        T GetHandle() const { return reinterpret_cast<T>(GetHandleImpl()); }

        virtual uint32_t GetBindingCount() const = 0;
        virtual const DescriptorSetLayoutDesc& GetDesc() const = 0;

    protected:
        virtual void* GetHandleImpl() const = 0;
    };

    // -------------------------------------------------------------------------
    // Descriptor set
    // -------------------------------------------------------------------------

    class RHIDescriptorSet
    {
    public:
        virtual ~RHIDescriptorSet() = default;

        template<typename T>
        T GetHandle() const { return reinterpret_cast<T>(GetHandleImpl()); }

        // Bind конкретных ресурсов по binding slot
        virtual void BindBuffer(uint32_t binding, const RHIBuffer*  buffer)   = 0;
        virtual void BindTexture(uint32_t binding, const RHITexture* texture) = 0;
        // для CombinedImageSampler
        virtual void BindTexture(uint32_t binding, const RHITexture* texture, const RHISampler* sampler) = 0;
        virtual void BindSampler(uint32_t binding, const RHISampler* sampler) = 0;

        // Применить все накопленные bind
        virtual void Update() = 0;

    protected:
        virtual void* GetHandleImpl() const = 0;
    };

} // namespace Flux
