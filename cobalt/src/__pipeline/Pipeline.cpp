#include <__pipeline/Pipeline.h>

#include <__context/DeviceSet.h>
#include <__meta/expect_size.h>
#include <__validation/result.h>


namespace cobalt
{
    Pipeline::Pipeline(  DeviceSet const& device, PipelineLayout const& layout, PipelineCreateInfo const& create_info )
        : device_ref_{ device }
        , layout_ref_{ layout }
        , bind_point_{ create_info.bind_point }
        , descriptor_sets_{ create_info.descriptor_sets.begin( ), create_info.descriptor_sets.end( ) }
    {
        validation::throw_on_bad_result(
            vkCreateGraphicsPipelines( device_ref_.logical( ), VK_NULL_HANDLE, 1,
                                       &create_info.create_info, nullptr, &pipeline_ ),
            "failed to create graphics pipeline!" );
    }


    Pipeline::~Pipeline( ) noexcept
    {
        if ( pipeline_ != VK_NULL_HANDLE )
        {
            vkDestroyPipeline( device_ref_.logical( ), pipeline_, nullptr );
        }
    }


    Pipeline::Pipeline( Pipeline&& other ) noexcept
        : device_ref_{ other.device_ref_ }
        , bind_point_{ other.bind_point_ }
        , descriptor_sets_{ std::move( other.descriptor_sets_ ) }
        , layout_ref_{ other.layout_ref_ }
        , pipeline_{ std::exchange( other.pipeline_, VK_NULL_HANDLE ) }
    {
        meta::expect_size<Pipeline, 64u>( );
    }


    VkPipeline Pipeline::handle( ) const
    {
        return pipeline_;
    }


    PipelineLayout const& Pipeline::layout( ) const
    {
        return layout_ref_;
    }


    VkPipelineBindPoint Pipeline::bind_point( ) const
    {
        return bind_point_;
    }


    std::span<DescriptorSet const* const> Pipeline::descriptor_sets( ) const
    {
        return descriptor_sets_;
    }

}
