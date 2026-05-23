#include "flpch.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanRenderPass.h"
#include "VulkanDescriptorSet.h"
#include "VulkanCommon.h"

namespace {

    using Flux::ShaderDataType;
    using Flux::PrimitiveTopology;
    using Flux::CullMode;
    using Flux::FillMode;
    using Flux::FrontFace;
    using Flux::CompareOp;
    using Flux::BlendFactor;
    using Flux::BlendOp;


    constexpr VkFormat ShaderDataTypeToVkFormat(ShaderDataType type)
    {
        using enum ShaderDataType;

        switch (type)
        {
        case Float:  return VK_FORMAT_R32_SFLOAT;
        case Float2: return VK_FORMAT_R32G32_SFLOAT;
        case Float3: return VK_FORMAT_R32G32B32_SFLOAT;
        case Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Int:    return VK_FORMAT_R32_SINT;
        case Int2:   return VK_FORMAT_R32G32_SINT;
        case Int3:   return VK_FORMAT_R32G32B32_SINT;
        case Int4:   return VK_FORMAT_R32G32B32A32_SINT;
		case UInt4:  return VK_FORMAT_R32G32B32A32_UINT;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown ShaderDataType");
        return VK_FORMAT_UNDEFINED;
    }

    constexpr VkPrimitiveTopology ToVkTopology(PrimitiveTopology t)
    {
        using enum PrimitiveTopology;

        switch (t)
        {
        case TriangleList:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case LineList:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case LineStrip:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PointList:     return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown PrimitiveTopology");
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }

    constexpr VkCullModeFlags ToVkCullMode(CullMode m)
    {
        using enum CullMode;

        switch (m)
        {
        case None:  return VK_CULL_MODE_NONE;
        case Front: return VK_CULL_MODE_FRONT_BIT;
        case Back:  return VK_CULL_MODE_BACK_BIT;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown CullMode");
        return VK_CULL_MODE_BACK_BIT;
    }

    constexpr VkPolygonMode ToVkFillMode(FillMode m)
    {
        using enum FillMode;

        switch (m)
        {
        case Solid:     return VK_POLYGON_MODE_FILL;
        case Wireframe: return VK_POLYGON_MODE_LINE;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown FillMode");
        return VK_POLYGON_MODE_FILL;
    }

    constexpr VkFrontFace ToVkFrontFace(FrontFace f)
    {
        using enum FrontFace;

        switch (f)
        {
        case CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case Clockwise:        return VK_FRONT_FACE_CLOCKWISE;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown FrontFace");
        return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }

    constexpr VkCompareOp ToVkCompareOp(CompareOp op)
    {
        using enum CompareOp;

        switch (op)
        {
        case Never:          return VK_COMPARE_OP_NEVER;
        case Less:           return VK_COMPARE_OP_LESS;
        case Equal:          return VK_COMPARE_OP_EQUAL;
        case LessOrEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
        case Greater:        return VK_COMPARE_OP_GREATER;
        case NotEqual:       return VK_COMPARE_OP_NOT_EQUAL;
        case GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case Always:         return VK_COMPARE_OP_ALWAYS;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown CompareOp");
        return VK_COMPARE_OP_LESS;
    }

    constexpr VkBlendFactor ToVkBlendFactor(BlendFactor f)
    {
        using enum BlendFactor;

        switch (f)
        {
        case Zero:             return VK_BLEND_FACTOR_ZERO;
        case One:              return VK_BLEND_FACTOR_ONE;
        case SrcColor:         return VK_BLEND_FACTOR_SRC_COLOR;
        case OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case DstColor:         return VK_BLEND_FACTOR_DST_COLOR;
        case OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case SrcAlpha:         return VK_BLEND_FACTOR_SRC_ALPHA;
        case OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case DstAlpha:         return VK_BLEND_FACTOR_DST_ALPHA;
        case OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown BlendFactor");
        return VK_BLEND_FACTOR_ONE;
    }

    constexpr VkBlendOp ToVkBlendOp(BlendOp op)
    {
        using enum BlendOp;

        switch (op)
        {
        case Add:             return VK_BLEND_OP_ADD;
        case Subtract:        return VK_BLEND_OP_SUBTRACT;
        case ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
        case Min:             return VK_BLEND_OP_MIN;
        case Max:             return VK_BLEND_OP_MAX;
        }

        FL_CONSTEXPR_ASSERT(false, "Unknown BlendOp");
        return VK_BLEND_OP_ADD;
    }

} // anonymous namespace

namespace Flux {

    // -------------------------------------------------------------------------
    // Constructor
    // -------------------------------------------------------------------------

    VulkanPipeline::VulkanPipeline(VkDevice device, const PipelineDesc& desc)
        : m_Device(device), m_Desc(desc)
    {
        CreatePipelineLayout();

        if (desc.Type == PipelineType::Compute)
            CreateComputePipeline();
        else
            CreateGraphicsPipeline();
    }

    VulkanPipeline::~VulkanPipeline()
    {
        vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
        vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
    }

    // -------------------------------------------------------------------------
    // Pipeline layout
    // -------------------------------------------------------------------------

    void VulkanPipeline::CreatePipelineLayout()
    {
        std::vector<VkDescriptorSetLayout> vkLayouts;
        for (const auto* layout : m_Desc.DescriptorSetLayouts)
            if (layout) vkLayouts.emplace_back(static_cast<VkDescriptorSetLayout>(layout->GetHandle()));

        VkPipelineLayoutCreateInfo info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        info.setLayoutCount = static_cast<uint32_t>(vkLayouts.size());
        info.pSetLayouts = vkLayouts.empty() ? nullptr : vkLayouts.data();

        VkPushConstantRange pushRange{};
        if (m_Desc.PushConstants.Size > 0)
        {
            pushRange.stageFlags = GetShaderStageFlags(m_Desc.PushConstants.Stage);
            pushRange.offset = m_Desc.PushConstants.Offset;
            pushRange.size = m_Desc.PushConstants.Size;
            info.pushConstantRangeCount = 1;
            info.pPushConstantRanges = &pushRange;
        }

        FL_CORE_ASSERT(vkCreatePipelineLayout(m_Device, &info, nullptr, &m_PipelineLayout) == VK_SUCCESS,
            "Failed to create Pipeline Layout");
    }

    // -------------------------------------------------------------------------
    // Graphics pipeline
    // -------------------------------------------------------------------------

    void VulkanPipeline::CreateGraphicsPipeline()
    {
        FL_CORE_ASSERT(m_Desc.VertexShader, "Graphics pipeline requires VertexShader");
        FL_CORE_ASSERT(m_Desc.RenderPass, "Graphics pipeline requires RenderPass");

        // --- Shader stages ---
        std::vector<VkPipelineShaderStageCreateInfo> stages;

        {
            auto* vs = static_cast<const VulkanShader*>(m_Desc.VertexShader);
            VkPipelineShaderStageCreateInfo s{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
            s.stage = VK_SHADER_STAGE_VERTEX_BIT;
            s.module = static_cast<VkShaderModule>(vs->GetHandle());
            s.pName = vs->GetEntryPoint().c_str();
            stages.push_back(s);
        }

        if (!m_Desc.DepthOnly && m_Desc.FragmentShader)
        {
            auto* fs = static_cast<const VulkanShader*>(m_Desc.FragmentShader);
            VkPipelineShaderStageCreateInfo s{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
            s.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            s.module = static_cast<VkShaderModule>(fs->GetHandle());
            s.pName = fs->GetEntryPoint().c_str();
            stages.push_back(s);
        }

        // --- Vertex input ---
        // Если layout пустой (depth-only без геометрии) — не создаём binding
        const bool hasVertexInput = m_Desc.VertexLayout.GetStride() > 0;

        std::vector<VkVertexInputAttributeDescription> attrs;
        VkVertexInputBindingDescription binding{};

        if (hasVertexInput)
        {
            uint32_t location = 0;
            for (const auto& el : m_Desc.VertexLayout)
            {
                VkVertexInputAttributeDescription a{};
                a.location = location++;
                a.binding = 0;
                a.format = ShaderDataTypeToVkFormat(el.Type);
                a.offset = static_cast<uint32_t>(el.Offset);
                attrs.push_back(a);
            }
            binding.binding = 0;
            binding.stride = m_Desc.VertexLayout.GetStride();
            binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        }

        VkPipelineVertexInputStateCreateInfo vertexInput{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        vertexInput.vertexBindingDescriptionCount = hasVertexInput ? 1 : 0;
        vertexInput.pVertexBindingDescriptions = hasVertexInput ? &binding : nullptr;
        vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrs.size());
        vertexInput.pVertexAttributeDescriptions = attrs.empty() ? nullptr : attrs.data();

        // --- Input assembly ---
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        inputAssembly.topology = ToVkTopology(m_Desc.Topology);
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // --- Viewport (dynamic) ---
        VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // --- Rasterizer — из RasterizerState ---
        const auto& rs = m_Desc.Rasterizer;
        VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        rasterizer.depthClampEnable = rs.DepthClamp ? VK_TRUE : VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = ToVkFillMode(rs.Fill);
        rasterizer.cullMode = ToVkCullMode(rs.Cull);
        rasterizer.frontFace = ToVkFrontFace(rs.Front);
        rasterizer.lineWidth = 1.0f;
        rasterizer.depthBiasEnable = (rs.DepthBiasConstant != 0.0f || rs.DepthBiasSlope != 0.0f);
        rasterizer.depthBiasConstantFactor = rs.DepthBiasConstant;
        rasterizer.depthBiasSlopeFactor = rs.DepthBiasSlope;

        // --- Multisampling ---
        VkPipelineMultisampleStateCreateInfo msaa{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        msaa.rasterizationSamples = GetSampleCount(m_Desc.Samples);
        msaa.sampleShadingEnable = VK_FALSE;

        // --- Depth stencil — из DepthStencilState ---
        const auto& ds = m_Desc.DepthStencil;
        VkPipelineDepthStencilStateCreateInfo depthStencil{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
        depthStencil.depthTestEnable = ds.DepthTest ? VK_TRUE : VK_FALSE;
        depthStencil.depthWriteEnable = ds.DepthWrite ? VK_TRUE : VK_FALSE;
        depthStencil.depthCompareOp = ToVkCompareOp(ds.DepthCompare); // больше не хардкод Less
        depthStencil.stencilTestEnable = ds.StencilTest ? VK_TRUE : VK_FALSE;

        // --- Color blend — из RenderTargetBlendState ---
        const auto& bl = m_Desc.Blend;
        VkPipelineColorBlendAttachmentState blendAttach{};
        blendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttach.blendEnable = bl.Enable ? VK_TRUE : VK_FALSE;
        blendAttach.srcColorBlendFactor = ToVkBlendFactor(bl.SrcColorFactor);
        blendAttach.dstColorBlendFactor = ToVkBlendFactor(bl.DstColorFactor);
        blendAttach.colorBlendOp = ToVkBlendOp(bl.ColorOp);
        blendAttach.srcAlphaBlendFactor = ToVkBlendFactor(bl.SrcAlphaFactor);
        blendAttach.dstAlphaBlendFactor = ToVkBlendFactor(bl.DstAlphaFactor);
        blendAttach.alphaBlendOp = ToVkBlendOp(bl.AlphaOp);

        uint32_t colorAttachCount = m_Desc.DepthOnly ? 0 : m_Desc.RenderPass->GetColorAttachmentCount();

        // Один blendAttach на каждый color attachment renderpass-а
        std::vector<VkPipelineColorBlendAttachmentState> blendAttachments(colorAttachCount, blendAttach);

        VkPipelineColorBlendStateCreateInfo colorBlend{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        colorBlend.logicOpEnable = VK_FALSE;
        colorBlend.attachmentCount = colorAttachCount;
        colorBlend.pAttachments = blendAttachments.empty() ? nullptr : blendAttachments.data();

        // --- Dynamic state ---
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };
        if (rasterizer.depthBiasEnable)
            dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);

        VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // --- Create ---
        VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        pipelineInfo.stageCount = static_cast<uint32_t>(stages.size());
        pipelineInfo.pStages = stages.data();
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &msaa;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlend;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.renderPass = static_cast<VkRenderPass>(m_Desc.RenderPass->GetHandle());
        pipelineInfo.subpass = 0;

        FL_CORE_ASSERT(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1,
            &pipelineInfo, nullptr, &m_Pipeline) == VK_SUCCESS,
            "Failed to create Graphics Pipeline");

        FL_CORE_INFO("Created Vulkan Graphics Pipeline");
    }

    // -------------------------------------------------------------------------
    // Compute pipeline
    // -------------------------------------------------------------------------

    void VulkanPipeline::CreateComputePipeline()
    {
        FL_CORE_ASSERT(m_Desc.ComputeShader, "Compute pipeline requires ComputeShader");

        auto* cs = static_cast<const VulkanShader*>(m_Desc.ComputeShader);

        VkPipelineShaderStageCreateInfo stage{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        stage.module = static_cast<VkShaderModule>(cs->GetHandle());
        stage.pName = cs->GetEntryPoint().c_str();

        VkComputePipelineCreateInfo info{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
        info.stage = stage;
        info.layout = m_PipelineLayout;

        FL_CORE_ASSERT(vkCreateComputePipelines(m_Device, VK_NULL_HANDLE, 1,
            &info, nullptr, &m_Pipeline) == VK_SUCCESS,
            "Failed to create Compute Pipeline");

        FL_CORE_INFO("Created Vulkan Compute Pipeline");
    }

} // namespace Flux
