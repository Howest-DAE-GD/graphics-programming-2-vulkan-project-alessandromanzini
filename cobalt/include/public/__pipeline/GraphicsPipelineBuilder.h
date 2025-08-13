#ifndef GRAPHICSPIPELINEBUILDER_H
#define GRAPHICSPIPELINEBUILDER_H

#include "Pipeline.h"

#include <__shader/ShaderModule.h>

#include <vulkan/vulkan_core.h>

#include <span>
#include <vector>


namespace cobalt
{
    class DescriptorSetLayout;
}

namespace cobalt::builder
{
    class GraphicsPipelineBuilder final
    {
    public:
        GraphicsPipelineBuilder( );
        ~GraphicsPipelineBuilder( ) noexcept = default;

        GraphicsPipelineBuilder( const GraphicsPipelineBuilder& )                = delete;
        GraphicsPipelineBuilder( GraphicsPipelineBuilder&& ) noexcept            = delete;
        GraphicsPipelineBuilder& operator=( const GraphicsPipelineBuilder& )     = delete;
        GraphicsPipelineBuilder& operator=( GraphicsPipelineBuilder&& ) noexcept = delete;

        GraphicsPipelineBuilder& set_binding_description(
            VkVertexInputBindingDescription, std::span<VkVertexInputAttributeDescription const> );

        GraphicsPipelineBuilder& add_color_attachment_description(
            VkPipelineColorBlendAttachmentState const& attachment_state, VkFormat color_image_format );

        GraphicsPipelineBuilder& set_depth_image_description( VkFormat depth_image_format );
        GraphicsPipelineBuilder& set_depth_stencil_mode( VkBool32 depth_testing, VkBool32 depth_writing, VkCompareOp compare_op );

        GraphicsPipelineBuilder& add_shader_module( shader::ShaderModule&& shader, VkSpecializationInfo const* = nullptr,
                                                    char const* entry_point = "main" );

        GraphicsPipelineBuilder& set_dynamic_state( std::span<VkDynamicState const> dynamic_states );

        GraphicsPipelineBuilder& set_cull_mode( VkCullModeFlags );

        Pipeline build( DeviceSet const&, PipelineLayout const& ) const;

    private:
        VkPipelineInputAssemblyStateCreateInfo input_assembly_{};
        VkPipelineRasterizationStateCreateInfo rasterization_{};
        VkPipelineMultisampleStateCreateInfo multisampling_{};
        VkPipelineVertexInputStateCreateInfo vertex_input_info_{};
        VkPipelineColorBlendStateCreateInfo color_blend_state_{};
        VkPipelineViewportStateCreateInfo viewport_state_{};
        VkPipelineDynamicStateCreateInfo dynamic_state_{};

        std::vector<VkVertexInputBindingDescription> vertex_bindings_desc_{};
        std::vector<VkVertexInputAttributeDescription> vertex_attributes_desc_{};

        std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments_{};
        std::vector<VkFormat> color_image_formats_{};

        VkPipelineDepthStencilStateCreateInfo depth_stencil_{};
        VkFormat depth_image_format_{};

        std::vector<VkDynamicState> dynamic_states_{};

        std::vector<shader::ShaderModule> shader_modules_{};
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages_{};

    };

}


#endif //!GRAPHICSPIPELINEBUILDER_H
