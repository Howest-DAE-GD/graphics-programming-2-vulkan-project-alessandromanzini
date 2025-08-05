#include <__pipeline/GraphicsPipeline.h>

#include <log.h>
#include <__context/DeviceSet.h>
#include <__validation/result.h>


namespace cobalt
{
    GraphicsPipeline::GraphicsPipeline( DeviceSet const& device, GraphicsPipelineCreateInfo const& create_info )
        : Pipeline{ device }
    {
        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.pVertexBindingDescriptions      = &create_info.binding_description.vertex_desc;
        vertex_input_info.vertexBindingDescriptionCount   = 1;
        vertex_input_info.pVertexAttributeDescriptions    = create_info.binding_description.attributes_desc.data( );
        vertex_input_info.vertexAttributeDescriptionCount =
                static_cast<uint32_t>( create_info.binding_description.attributes_desc.size( ) );

        // The second structure references the array of structures for all the frame buffers and allows you to set blend
        // constants that you can use as blend factors.
        VkPipelineColorBlendStateCreateInfo color_blending{};
        color_blending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending.logicOpEnable     = VK_FALSE;
        color_blending.logicOp           = VK_LOGIC_OP_COPY; // Optional
        color_blending.attachmentCount   = 1;
        color_blending.pAttachments      = &create_info.color_blend_attachment;
        color_blending.blendConstants[0] = 0.0f; // Optional
        color_blending.blendConstants[1] = 0.0f; // Optional
        color_blending.blendConstants[2] = 0.0f; // Optional
        color_blending.blendConstants[3] = 0.0f; // Optional

        // Dynamic state represents the states that can be changed without recreating the pipeline.
        // This setup will cause the configuration of these values to be ignored, and you will be able (and required) to
        // specify the data at drawing time. This results in a more flexible setup and is very common for things like
        // viewport and scissor state, which would result in a more complex setup when being baked into the pipeline state.
        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount  = 1;

        constexpr std::array dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = static_cast<uint32_t>( dynamic_states.size( ) );
        dynamic_state.pDynamicStates    = dynamic_states.data( );

        // You can use uniform values in shaders, which are globals similar to dynamic state variables that can be changed
        // at drawing time to alter the behavior of your shaders without having to recreate them. They are commonly used to
        // pass the transformation matrix to the vertex shader, or to create texture samplers in the fragment shader.
        VkPipelineLayoutCreateInfo pipeline_layout_info{};
        pipeline_layout_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 1;
        pipeline_layout_info.pSetLayouts    = create_info.descriptor_set_layouts.data( );
        create_layout( pipeline_layout_info );

        // We can now combine all the structures and objects to create the graphics pipeline.
        // 1. Shader stages: the shader modules that define the functionality of the programmable stages of the
        // graphics pipeline.
        // 2. Fixed-function state: all the structures that define the fixed-function stages of the pipeline, like input
        // assembly, rasterizer, viewport and color blending.
        // 3. Pipeline layout: the uniform and push values referenced by the shader that can be updated at draw time.
        // 4. Pipeline rendering: specifies the dynamic rendering information, such as the color and depth formats of the swapchain
        // which we will use when dynamically recording commands.

        // 1. We start by referencing the array of VkPipelineShaderStageCreateInfo structs.
        VkGraphicsPipelineCreateInfo pipeline_info{};
        pipeline_info.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = 2;
        pipeline_info.pStages    = create_info.shader_stages.data( );

        // 2. Then we reference all the structures describing the fixed-function stage.
        pipeline_info.pVertexInputState   = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &create_info.input_assembly;
        pipeline_info.pViewportState      = &viewport_state;
        pipeline_info.pRasterizationState = &create_info.rasterization;
        pipeline_info.pMultisampleState   = &create_info.multisampling;
        pipeline_info.pColorBlendState    = &color_blending;
        pipeline_info.pDynamicState       = &dynamic_state;

        // 3. After that comes the pipeline layout, which is a Vulkan handle rather than a struct pointer.
        pipeline_info.layout             = layout( );
        pipeline_info.pDepthStencilState = &create_info.depth_stencil;

        // 4. Finally, we specify the dynamic rendering information.
        VkPipelineRenderingCreateInfo pipeline_rendering_info{};
        pipeline_rendering_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipeline_rendering_info.colorAttachmentCount    = 1;
        pipeline_rendering_info.pColorAttachmentFormats = &create_info.swapchain_image_format;
        pipeline_rendering_info.depthAttachmentFormat   = create_info.depth_image_format;

        pipeline_info.pNext = &pipeline_rendering_info;

        // The vkCreateGraphicsPipelines is designed to take multiple VkGraphicsPipelineCreateInfo objects and create
        // multiple VkPipeline objects in a single call.
        // The second parameter references an optional VkPipelineCache object. A pipeline cache can be used to store and
        // reuse data relevant to pipeline creation across multiple calls to vkCreateGraphicsPipelines.
        create_handle( pipeline_info );
    }

}
