#include <__pipeline/PipelineLayout.h>

#include <__context/DeviceSet.h>
#include <__descriptor/DescriptorSetLayout.h>
#include <__validation/result.h>


namespace cobalt
{
    PipelineLayout::PipelineLayout( DeviceSet const& device, std::span<DescriptorSetLayout const* const> desc_layouts,
                                    std::span<VkPushConstantRange const> push_constant_ranges )
        : device_ref_{ device }
        , descriptor_layouts_{ desc_layouts.begin( ), desc_layouts.end( ) }
    {
        std::vector<VkDescriptorSetLayout> raw_layouts( descriptor_layouts_.size( ) );
        std::ranges::transform(
            descriptor_layouts_, raw_layouts.begin( ),
            []( DescriptorSetLayout const* layout ) -> VkDescriptorSetLayout { return layout->handle( ); } );

        VkPipelineLayoutCreateInfo const layout_create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = static_cast<uint32_t>( raw_layouts.size( ) ),
            .pSetLayouts = raw_layouts.data( ),
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


    std::span<DescriptorSetLayout const* const> PipelineLayout::descriptor_layouts( ) const noexcept
    {
        return descriptor_layouts_;
    }

}
