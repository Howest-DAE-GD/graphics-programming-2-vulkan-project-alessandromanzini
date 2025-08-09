#ifndef QUEUE_H
#define QUEUE_H

#include <__memory/Resource.h>

#include <__synchronization/PresentInfo.h>
#include <__synchronization/SubmitInfo.h>

#include <vulkan/vulkan_core.h>


namespace cobalt
{
    namespace sync
    {
        class Fence;
    }
    class DeviceSet;
}

namespace cobalt
{
    class Queue final : public memory::Resource
    {
    public:
        explicit Queue( DeviceSet const&, uint32_t queue_family_index, uint32_t queue_index );
        ~Queue( ) noexcept override = default;

        Queue( const Queue& )                = delete;
        Queue( Queue&& ) noexcept            = delete;
        Queue& operator=( const Queue& )     = delete;
        Queue& operator=( Queue&& ) noexcept = delete;

        [[nodiscard]] VkQueue handle( ) const;
        [[nodiscard]] uint32_t queue_family_index( ) const;

        void submit( sync::SubmitInfo const&, sync::Fence const* fence = nullptr ) const;
        void submit( VkSubmitInfo const&, sync::Fence const* fence = nullptr ) const;

        void wait_idle( ) const;

        void submit_and_wait( sync::SubmitInfo const&, sync::Fence const* = nullptr ) const;
        void submit_and_wait( VkSubmitInfo const&, sync::Fence const* = nullptr ) const;

        VkResult present( sync::PresentInfo const& ) const;

    private:
        uint32_t const queue_family_index_{ UINT32_MAX };
        uint32_t const queue_index_{ UINT32_MAX };

        VkQueue queue_{ VK_NULL_HANDLE };

    };

}


#endif //!QUEUE_H
