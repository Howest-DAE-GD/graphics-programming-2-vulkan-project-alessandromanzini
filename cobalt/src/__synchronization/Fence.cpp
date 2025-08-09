#include <__synchronization/Fence.h>

#include <__context/DeviceSet.h>
#include <__validation/result.h>


namespace cobalt::sync
{
    Fence::Fence( DeviceSet const& device, VkFenceCreateFlags const create_flags ) noexcept
        : device_ref_( device )
    {
        VkFenceCreateInfo const fence_info{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = create_flags
        };
        validation::throw_on_bad_result(
            vkCreateFence( device_ref_.logical( ), &fence_info, nullptr, &fence_ ),
            "failed to create fence!" );
    }


    Fence::~Fence( ) noexcept
    {
        vkDestroyFence( device_ref_.logical( ), fence_, nullptr );
    }


    VkFence Fence::handle( ) const noexcept
    {
        return fence_;
    }


    void Fence::wait( ) const noexcept
    {
        device_ref_.wait_for_fence( fence_ );
    }


    void Fence::reset( ) const noexcept
    {
        device_ref_.reset_fence( fence_ );
    }

}
