#include <__pipeline/PipelineLayout.h>

#include <__context/DeviceSet.h>
#include <__descriptor/DescriptorSet.h>
#include <__validation/result.h>


namespace cobalt
{
    PipelineLayout::PipelineLayout( DeviceSet const& device, std::span<DescriptorSet const* const> descriptor_sets,
                                    std::span<VkPushConstantRange const> push_constant_ranges )
        : device_ref_{ device }
        , descriptor_sets_{ descriptor_sets.begin( ), descriptor_sets.end( ) }
    {
        std::ranges::sort( descriptor_sets_,
                           []( DescriptorSet const* a, DescriptorSet const* b )
                               {
                                   return a->offset( ) < b->offset( );
                               } );

        std::vector<VkDescriptorSetLayout> raw_layouts( descriptor_sets_.size( ) );
        std::ranges::transform(
            descriptor_sets_, raw_layouts.begin( ),
            []( DescriptorSet const* set ) -> VkDescriptorSetLayout { return set->layout( ).handle( ); } );

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


    std::span<DescriptorSet const* const> PipelineLayout::descriptor_sets( ) const noexcept
    {
        return descriptor_sets_;
    }

}
