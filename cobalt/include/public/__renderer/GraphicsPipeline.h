#ifndef GRAPHICSPIPELINE_H
#define GRAPHICSPIPELINE_H

#include <__memory/Resource.h>

#include <__renderer/BindingDescription.h>

#include <vulkan/vulkan_core.h>

#include <vector>
#include <__init/InitWizard.h>


namespace cobalt
{
    class DeviceSet;
}

namespace cobalt
{
    struct GraphicsPipelineCreateInfo
    {
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages{};
        std::vector<VkDescriptorSetLayout> descriptor_set_layouts{};
        BindingDescription binding_description{};
        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        VkPipelineRasterizationStateCreateInfo rasterization{};
        VkPipelineMultisampleStateCreateInfo multisampling{};
        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        VkPipelineDepthStencilStateCreateInfo depth_stencil{};
        VkFormat swapchain_image_format{ VK_FORMAT_UNDEFINED };
        VkFormat depth_image_format{ VK_FORMAT_UNDEFINED };
    };


    class GraphicsPipeline final : public memory::Resource
    {
    public:
        explicit GraphicsPipeline( DeviceSet const&, GraphicsPipelineCreateInfo const& );
        ~GraphicsPipeline( ) noexcept override;

        GraphicsPipeline( const GraphicsPipeline& )                = delete;
        GraphicsPipeline( GraphicsPipeline&& ) noexcept            = delete;
        GraphicsPipeline& operator=( const GraphicsPipeline& )     = delete;
        GraphicsPipeline& operator=( GraphicsPipeline&& ) noexcept = delete;

        [[nodiscard]] VkPipeline handle( ) const;
        [[nodiscard]] VkPipelineLayout layout( ) const;

    private:
        DeviceSet const& device_ref_;

        VkPipelineLayout pipeline_layout_{ VK_NULL_HANDLE };
        VkPipeline graphics_pipeline_{ VK_NULL_HANDLE };

    };

}


#endif //!GRAPHICSPIPELINE_H
