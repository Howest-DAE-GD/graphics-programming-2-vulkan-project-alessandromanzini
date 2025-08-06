#include <__context/Queue.h>

#include <__context/DeviceSet.h>
#include <__validation/result.h>


namespace cobalt
{
    Queue::Queue( DeviceSet const& device, uint32_t const queue_family_index, uint32_t const queue_index )
        : queue_family_index_{ queue_family_index }
        , queue_index_{ queue_index }
    {
        vkGetDeviceQueue( device.logical( ), queue_family_index_, queue_index_, &queue_ );
    }


    VkQueue Queue::handle( ) const
    {
        return queue_;
    }


    void Queue::submit( VkSubmitInfo2 const& submit_info, VkFence const fence ) const
    {
        validation::throw_on_bad_result(
            vkQueueSubmit2( queue_, 1, &submit_info, fence ), "failed to submit queue!" );
    }


    void Queue::submit( VkSubmitInfo const& submit_info, VkFence const fence ) const
    {
        validation::throw_on_bad_result(
            vkQueueSubmit( queue_, 1, &submit_info, fence ), "failed to submit queue!" );
    }


    void Queue::wait_idle( ) const
    {
        vkQueueWaitIdle( queue_ );
    }


    void Queue::submit_and_wait( VkSubmitInfo2 const& submit_info, VkFence const fence ) const
    {
        submit( submit_info, fence );
        wait_idle( );
    }


    void Queue::submit_and_wait( VkSubmitInfo const& submit_info, VkFence const fence ) const
    {
        submit( submit_info, fence );
        wait_idle( );
    }


    VkResult Queue::present( VkPresentInfoKHR const& present_info ) const
    {
        return vkQueuePresentKHR( queue_, &present_info );
    }

}
