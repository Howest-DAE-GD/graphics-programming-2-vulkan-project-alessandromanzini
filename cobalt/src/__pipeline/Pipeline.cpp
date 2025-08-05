#include <__pipeline/Pipeline.h>

#include <__context/DeviceSet.h>
#include <__validation/result.h>


namespace cobalt
{
    Pipeline::~Pipeline( ) noexcept
    {
        if ( pipeline_ != VK_NULL_HANDLE )
        {
            vkDestroyPipeline( device_ref_.logical( ), pipeline_, nullptr );
        }
        if ( layout_ != VK_NULL_HANDLE )
        {
            vkDestroyPipelineLayout( device_ref_.logical( ), layout_, nullptr );
        }
    }


    VkPipeline Pipeline::handle( ) const
    {
        return pipeline_;
    }


    VkPipelineLayout Pipeline::layout( ) const
    {
        return layout_;
    }


    Pipeline::Pipeline( DeviceSet const& device ) : device_ref_{ device } { }


    void Pipeline::create_layout( VkPipelineLayoutCreateInfo const& layout_create_info )
    {
        validation::throw_on_bad_result(
            vkCreatePipelineLayout( device_ref_.logical( ), &layout_create_info, nullptr, &layout_ ),
            "Failed to create pipeline layout!" );
    }


    void Pipeline::create_handle( VkGraphicsPipelineCreateInfo const& pipeline_create_info )
    {
        validation::throw_on_bad_result(
            vkCreateGraphicsPipelines( device_ref_.logical( ), VK_NULL_HANDLE, 1,
                                       &pipeline_create_info, nullptr, &pipeline_ ),
            "Failed to create graphics pipeline!" );
    }
}
