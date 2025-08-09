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
    class Fence final : public memory::Resource
    {
    public:
        explicit Fence( DeviceSet const&, VkFenceCreateFlags create_flags = 0 ) noexcept;
        ~Fence( ) noexcept override;

        Fence( const Fence& )                = delete;
        Fence( Fence&& ) noexcept            = delete;
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
