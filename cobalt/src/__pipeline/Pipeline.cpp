#include <__pipeline/Pipeline.h>

#include <__context/DeviceSet.h>
#include <__meta/expect_size.h>
#include <__validation/result.h>


namespace cobalt
{
    Pipeline::Pipeline( DeviceSet const& device, PipelineLayout const& layout, VkGraphicsPipelineCreateInfo const& create_info )
        : device_ref_{ device }
        , layout_ref_{ layout }
    {
        validation::throw_on_bad_result(
            vkCreateGraphicsPipelines( device_ref_.logical( ), VK_NULL_HANDLE, 1,
                                       &create_info, nullptr, &pipeline_ ),
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
        , layout_ref_{ other.layout_ref_ }
        , pipeline_{ std::exchange( other.pipeline_, VK_NULL_HANDLE ) }
    {
        meta::expect_size<Pipeline, 32u>( );
    }


    VkPipeline Pipeline::handle( ) const
    {
        return pipeline_;
    }


    PipelineLayout const& Pipeline::layout( ) const
    {
        return layout_ref_;
    }

}
