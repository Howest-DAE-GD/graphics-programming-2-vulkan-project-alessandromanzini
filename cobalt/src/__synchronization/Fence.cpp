#include <__synchronization/Fence.h>

#include <__context/DeviceSet.h>
#include <__meta/expect_size.h>
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
        if ( fence_ != VK_NULL_HANDLE )
        {
            vkDestroyFence( device_ref_.logical( ), fence_, nullptr );
        }
    }


    Fence::Fence( Fence&& other ) noexcept
        : device_ref_{ other.device_ref_ }
        , fence_{ other.fence_ }
    {
        meta::expect_size<Fence, 24u>( );
        other.fence_ = VK_NULL_HANDLE;
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
