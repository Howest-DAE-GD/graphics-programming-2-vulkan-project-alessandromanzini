#include <__pipeline/PipelineLayout.h>

#include <__context/DeviceSet.h>
#include <__validation/result.h>


namespace cobalt
{
    PipelineLayout::PipelineLayout( DeviceSet const& device, std::span<VkDescriptorSetLayout const> desc_layouts,
            std::span<VkPushConstantRange const> push_constant_ranges )
        : device_ref_{ device }
    {
        VkPipelineLayoutCreateInfo const layout_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>( desc_layouts.size( ) ),
            .pSetLayouts = desc_layouts.data( ),
            .pushConstantRangeCount = static_cast<uint32_t>( push_constant_ranges.size( ) ),
            .pPushConstantRanges = push_constant_ranges.data( ),
        };

        validation::throw_on_bad_result(
            vkCreatePipelineLayout( device_ref_.logical( ), &layout_create_info, nullptr, &layout_ ),
            "failed to create pipeline layout!" );
    }


    PipelineLayout::~PipelineLayout( ) noexcept
    {
        if ( layout_ != VK_NULL_HANDLE )
        {
            vkDestroyPipelineLayout( device_ref_.logical( ), layout_, nullptr );
        }
    }


    VkPipelineLayout PipelineLayout::handle( ) const noexcept
    {
        return layout_;
    }

}
