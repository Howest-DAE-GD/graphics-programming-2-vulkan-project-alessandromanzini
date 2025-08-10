#include <__synchronization/Semaphore.h>

#include <__context/DeviceSet.h>
#include <__meta/expect_size.h>
#include <__validation/result.h>


namespace cobalt::sync
{
    Semaphore::Semaphore( DeviceSet const& device, VkSemaphoreCreateFlags const create_flags ) noexcept
        : device_ref_{ device }
    {
        VkSemaphoreCreateInfo const semaphore_info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .flags = create_flags
        };
        validation::throw_on_bad_result(
            vkCreateSemaphore( device_ref_.logical( ), &semaphore_info, nullptr, &semaphore_ ),
            "failed to create image semaphore!" );
    }


    Semaphore::~Semaphore( ) noexcept
    {
        if ( semaphore_ != VK_NULL_HANDLE )
        {
            vkDestroySemaphore( device_ref_.logical( ), semaphore_, nullptr );
        }
    }


    Semaphore::Semaphore( Semaphore&& other ) noexcept
        : device_ref_{ other.device_ref_ }
        , semaphore_{ other.semaphore_ }
    {
        meta::expect_size<Semaphore, 24u>( );
        other.semaphore_ = VK_NULL_HANDLE;
    }


    VkSemaphore Semaphore::handle( ) const noexcept
    {
        return semaphore_;
    }


    VkSemaphoreSubmitInfo Semaphore::make_submit_info( VkPipelineStageFlags const mask, uint32_t const device_idx ) const noexcept
    {
        return {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = semaphore_,
            .stageMask = mask,
            .deviceIndex = device_idx,
            .value = 0 // 0 for binary semaphore, non-zero for timeline
        };
    }

}
