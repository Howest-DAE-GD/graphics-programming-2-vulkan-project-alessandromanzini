#ifndef GRAPHICSPIPELINE_H
#define GRAPHICSPIPELINE_H

#include <__pipeline/Pipeline.h>

#include <__render/BindingDescription.h>

#include <vulkan/vulkan_core.h>

#include <optional>
#include <vector>


namespace cobalt
{
    struct GraphicsPipelineCreateInfo
    {
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages{};
        std::vector<VkDescriptorSetLayout> descriptor_set_layouts{};
        std::vector<VkPushConstantRange> push_constant_ranges{};
        BindingDescription binding_description{};
        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        VkPipelineRasterizationStateCreateInfo rasterization{};
        VkPipelineMultisampleStateCreateInfo multisampling{};
        std::optional<VkPipelineColorBlendAttachmentState> color_blend_attachment{};
        VkPipelineDepthStencilStateCreateInfo depth_stencil{};
        VkFormat swapchain_image_format{ VK_FORMAT_UNDEFINED };
        VkFormat depth_image_format{ VK_FORMAT_UNDEFINED };
    };


    class GraphicsPipeline final : public Pipeline
    {
    public:
        explicit GraphicsPipeline( DeviceSet const&, GraphicsPipelineCreateInfo const& );
        ~GraphicsPipeline( ) noexcept override = default;

        GraphicsPipeline( const GraphicsPipeline& )                = delete;
        GraphicsPipeline( GraphicsPipeline&& ) noexcept            = delete;
        GraphicsPipeline& operator=( const GraphicsPipeline& )     = delete;
        GraphicsPipeline& operator=( GraphicsPipeline&& ) noexcept = delete;

    };

}


#endif //!GRAPHICSPIPELINE_H
