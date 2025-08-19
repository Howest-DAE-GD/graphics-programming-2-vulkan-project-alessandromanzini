#include <__pipeline/GraphicsPipelineBuilder.h>


namespace cobalt::builder
{
    GraphicsPipelineBuilder::GraphicsPipelineBuilder( )
        : input_assembly_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        }
        , rasterization_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .lineWidth = 1.0f,
        }
        , multisampling_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
        }
        , vertex_input_info_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = nullptr,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions = nullptr,
        }
        , color_blend_state_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
        }
        , viewport_state_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        }
        , dynamic_state_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
        } { }


    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_binding_description(
        VkVertexInputBindingDescription binding_desc, std::span<VkVertexInputAttributeDescription const> attributes_desc )
    {
        vertex_bindings_desc_.emplace_back( binding_desc );
        vertex_input_info_.vertexBindingDescriptionCount = static_cast<uint32_t>( vertex_bindings_desc_.size( ) );
        vertex_input_info_.pVertexBindingDescriptions    = vertex_bindings_desc_.data( );

        vertex_attributes_desc_.reserve( attributes_desc.size( ) );
        std::ranges::copy( attributes_desc, std::back_inserter( vertex_attributes_desc_ ) );
        vertex_input_info_.vertexAttributeDescriptionCount = static_cast<uint32_t>( vertex_attributes_desc_.size( ) );
        vertex_input_info_.pVertexAttributeDescriptions    = vertex_attributes_desc_.data( );

        return *this;
    }


    GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_color_attachment_description(
        VkPipelineColorBlendAttachmentState const& attachment_state, VkFormat color_image_format )
    {
        color_blend_attachments_.emplace_back( attachment_state );
        color_image_formats_.emplace_back( color_image_format );

        color_blend_state_.attachmentCount = static_cast<uint32_t>( color_blend_attachments_.size( ) );
        color_blend_state_.pAttachments    = color_blend_attachments_.data( );

        return *this;
    }


    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_depth_image_description( VkFormat const depth_image_format )
    {
        depth_image_format_ = depth_image_format;

        return *this;
    }


    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_depth_stencil_mode(
        VkBool32 const depth_testing, VkBool32 const depth_writing, VkCompareOp const compare_op )
    {
        depth_stencil_.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_.depthTestEnable       = depth_testing;
        depth_stencil_.depthWriteEnable      = depth_writing;
        depth_stencil_.depthCompareOp        = compare_op;
        depth_stencil_.depthBoundsTestEnable = VK_FALSE;
        depth_stencil_.stencilTestEnable     = VK_FALSE;
        depth_stencil_.front                 = {};
        depth_stencil_.back                  = {};
        depth_stencil_.minDepthBounds        = 0.0f;
        depth_stencil_.maxDepthBounds        = 1.0f;

        return *this;
    }


    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_depth_bias(
        float const constant_factor, float const /* clamp */, float const slope_factor)
    {
        //todo: manage depth clamp
        rasterization_.depthBiasEnable         = VK_TRUE;
        //rasterization_.depthClampEnable        = VK_TRUE;
        rasterization_.depthBiasConstantFactor = constant_factor;
        //rasterization_.depthBiasClamp          = clamp;
        rasterization_.depthBiasSlopeFactor    = slope_factor;

        return *this;
    }


    GraphicsPipelineBuilder& GraphicsPipelineBuilder::add_shader_module( shader::ShaderModule&& shader,
                                                                         VkSpecializationInfo const* specialization_info,
                                                                         char const* entry_point )
    {
        shader_stages_.emplace_back(
            VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = shader.stage( ),
                .module = shader.handle( ),
                .pName = entry_point,
                .pSpecializationInfo = specialization_info,
            } );
        shader_modules_.emplace_back( std::move( shader ) );

        return *this;
    }


    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_dynamic_state( std::span<VkDynamicState const> dynamic_states )
    {
        dynamic_states_ = std::vector<VkDynamicState>{ dynamic_states.begin( ), dynamic_states.end( ) };

        dynamic_state_.dynamicStateCount = static_cast<uint32_t>( dynamic_states_.size( ) );
        dynamic_state_.pDynamicStates    = dynamic_states_.data( );

        return *this;
    }


    GraphicsPipelineBuilder& GraphicsPipelineBuilder::set_cull_mode( VkCullModeFlags const cullmode )
    {
        rasterization_.cullMode = cullmode;

        return *this;
    }


    Pipeline GraphicsPipelineBuilder::build(
        DeviceSet const& device, PipelineLayout const& layout, VkPipelineBindPoint const bind_point ) const
    {
        VkPipelineRenderingCreateInfo const pipeline_rendering_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = static_cast<uint32_t>( color_blend_attachments_.size( ) ),
            .pColorAttachmentFormats = color_image_formats_.data( ),
            .depthAttachmentFormat = depth_image_format_,
        };

        return Pipeline{
            device, layout,
            PipelineCreateInfo{
                .bind_point = bind_point,
                .create_info = VkGraphicsPipelineCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                    .pNext = &pipeline_rendering_info,

                    .stageCount = static_cast<uint32_t>( shader_stages_.size( ) ),
                    .pStages = shader_stages_.data( ),

                    .pVertexInputState = &vertex_input_info_,
                    .pInputAssemblyState = &input_assembly_,
                    .pViewportState = &viewport_state_,
                    .pRasterizationState = &rasterization_,
                    .pMultisampleState = &multisampling_,
                    .pDepthStencilState = &depth_stencil_,
                    .pColorBlendState = &color_blend_state_,
                    .pDynamicState = &dynamic_state_,

                    .layout = layout.handle( ),
                }
            }
        };
    }

}
