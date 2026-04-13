#include "flpch.h"

#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanBuffer.h"
#include "VulkanRenderpass.h"
#include "VulkanDescriptorSet.h"



namespace Flux {

    static VkFormat ShaderDataTypeToVkFormat(ShaderDataType type)
    {
        switch (type)
        {
        case ShaderDataType::Float:  return VK_FORMAT_R32_SFLOAT;
        case ShaderDataType::Float2: return VK_FORMAT_R32G32_SFLOAT;
        case ShaderDataType::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
        case ShaderDataType::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case ShaderDataType::Int:    return VK_FORMAT_R32_SINT;
        case ShaderDataType::Int2:   return VK_FORMAT_R32G32_SINT;
        case ShaderDataType::Int3:   return VK_FORMAT_R32G32B32_SINT;
        case ShaderDataType::Int4:   return VK_FORMAT_R32G32B32A32_SINT;
        }

        FL_CORE_ASSERT(false, "Unknown ShaderDataType!");
        return VK_FORMAT_UNDEFINED;
    }

    static VkPrimitiveTopology GetPrimitiveTopology(PrimitiveTopology topology)
    {
        switch (topology)
        {
        case PrimitiveTopology::TriangleList:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveTopology::LineList:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveTopology::PointList:     return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        }

        FL_CORE_ASSERT(false, "Unknown Primitive Topology!");
        return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    }

    static VkShaderStageFlags GetShaderStageFlags(ShaderStage stage)
    {
        VkShaderStageFlags flags = 0;

        if (HasFlag(stage, ShaderStage::Vertex))
            flags |= VK_SHADER_STAGE_VERTEX_BIT;

        if (HasFlag(stage, ShaderStage::Fragment))
            flags |= VK_SHADER_STAGE_FRAGMENT_BIT;

        if (HasFlag(stage, ShaderStage::Compute))
            flags |= VK_SHADER_STAGE_COMPUTE_BIT;

        // Если не установлен ни один флаг, можно вернуть 0 или ассерт
        if (flags == 0)
        {
            FL_CORE_ASSERT(false, "No Shader Stage specified!");
        }

        return flags;

    }

    VulkanPipeline::VulkanPipeline(VkDevice device, const PipelineDesc& desc)
        : m_Device(device), m_Desc(desc)
    {
        // сначала создаем Layout
        CreatePipelineLayout();

        // Создание графического конвейера
		// Первый этап - Input Assembly
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = GetPrimitiveTopology(desc.Topology);
        inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Shader stages
        auto* vertShader = static_cast<const VulkanShader*>(desc.VertexShader);
        auto* fragShader = static_cast<const VulkanShader*>(desc.FragmentShader);

        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertShader->GetHandle<VkShaderModule>();
        shaderStages[0].pName = vertShader->GetEntryPoint().c_str();

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShader->GetHandle<VkShaderModule>();
        shaderStages[1].pName = fragShader->GetEntryPoint().c_str();

        // Vertex input
        std::vector<VkVertexInputAttributeDescription> attributeDescs{};
        uint32_t location = 0;
        for (auto& element : desc.VertexLayout)
        {
            VkVertexInputAttributeDescription attr{};
            attr.location = location++;
            attr.binding = 0;
            attr.format = ShaderDataTypeToVkFormat(element.Type);
            attr.offset = static_cast<uint32_t>(element.Offset);
            attributeDescs.emplace_back(attr);
        }

        VkVertexInputBindingDescription bindingDesc{};
        bindingDesc.binding = 0;
        bindingDesc.stride = desc.VertexLayout.GetStride();
        bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInput.vertexBindingDescriptionCount = 1;
        vertexInput.pVertexBindingDescriptions = &bindingDesc;
        vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescs.size());
        vertexInput.pVertexAttributeDescriptions = attributeDescs.data();

        // Viewport — dynamic state
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        // Multisampling
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Depth Stencil
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = desc.DepthStencil.DepthTest ? VK_TRUE : VK_FALSE;
        depthStencil.depthWriteEnable = desc.DepthStencil.DepthWrite ? VK_TRUE : VK_FALSE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.stencilTestEnable = VK_FALSE;

        // Color Blending
        VkPipelineColorBlendAttachmentState blendAttachment{};
        blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blendAttachment.blendEnable = desc.Blend.Enable ? VK_TRUE : VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlend{};
        colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlend.logicOpEnable = VK_FALSE;
        colorBlend.attachmentCount = 1;
        colorBlend.pAttachments = &blendAttachment;

        //Dynamic State — viewport и scissor задаём в рантайме
        std::array<VkDynamicState, 2> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // Сам пайплайн
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlend;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_PipelineLayout;
        pipelineInfo.renderPass = desc.RenderPass->GetHandle<VkRenderPass>();
        pipelineInfo.subpass = 0;

        FL_CORE_ASSERT(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) == VK_SUCCESS,
            "Failed to create Graphics Pipeline");

        FL_CORE_INFO("Created Vulkan Pipeline");

    }

    VulkanPipeline::~VulkanPipeline()
    {
        vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
        vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
    }

    void VulkanPipeline::CreatePipelineLayout()
    {
        auto descriptorSetLayout = m_Desc.DescriptorSetLayout->GetHandle<VkDescriptorSetLayout>();

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = &descriptorSetLayout;
        

        auto pushConstants = m_Desc.pipelineLayoutDesc;
        if (pushConstants.Size > 0)
        {
            VkShaderStageFlags stageFlags = GetShaderStageFlags(pushConstants.Stage);

            VkPushConstantRange pushConstantRange{};
            pushConstantRange.stageFlags = stageFlags;
            pushConstantRange.offset = pushConstants.Offset;
            pushConstantRange.size = pushConstants.Size;

            layoutInfo.pushConstantRangeCount = 1;
            layoutInfo.pPushConstantRanges = &pushConstantRange;
        }
        else
        {
            layoutInfo.pushConstantRangeCount = 0;
            layoutInfo.pPushConstantRanges = nullptr;
        }


        FL_CORE_ASSERT(vkCreatePipelineLayout(m_Device, &layoutInfo, nullptr, &m_PipelineLayout) == VK_SUCCESS,
            "Failed to create Pipeline Layout");

    }

}