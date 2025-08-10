#ifndef FENCE_H
#define FENCE_H

#include <__memory/Resource.h>

#include <vulkan/vulkan_core.h>


namespace cobalt
{
    class DeviceSet;
}

namespace cobalt::sync
{
    /**
     * A fence is used when the host needs to know when the GPU has finished something, It is used to synchronize execution, for
     * ordering the execution on the CPU when the GPU signals.
     */

    class Fence final : public memory::Resource
    {
    public:
        explicit Fence( DeviceSet const&, VkFenceCreateFlags create_flags = 0 ) noexcept;
        ~Fence( ) noexcept override;

        Fence( Fence&& ) noexcept;
        Fence( const Fence& )                = delete;
        Fence& operator=( const Fence& )     = delete;
        Fence& operator=( Fence&& ) noexcept = delete;

        [[nodiscard]] VkFence handle( ) const noexcept;

        void wait( ) const noexcept;
        void reset( ) const noexcept;

    private:
        DeviceSet const& device_ref_;
        VkFence fence_{ VK_NULL_HANDLE };

    };

}


#endif //!FENCE_H
