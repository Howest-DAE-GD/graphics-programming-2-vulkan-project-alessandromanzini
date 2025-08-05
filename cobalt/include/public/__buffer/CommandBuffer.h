#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include <__memory/Resource.h>

#include <vulkan/vulkan_core.h>

#include "CommandOperator.h"


namespace cobalt
{
    class DeviceSet;
    class CommandPool;
    class Pipeline;
}

namespace cobalt
{
    class CommandBuffer final : memory::Resource
    {
        friend class CommandPool;

    public:
        ~CommandBuffer( ) noexcept override;

        CommandBuffer( CommandBuffer&& ) noexcept;
        CommandBuffer( const CommandBuffer& )                = delete;
        CommandBuffer& operator=( const CommandBuffer& )     = delete;
        CommandBuffer& operator=( CommandBuffer&& ) noexcept = delete;

        void unlock( ) const noexcept;
        void reset( VkCommandBufferResetFlags = 0 ) const;

        [[nodiscard]] CommandOperator command_operator( VkCommandBufferUsageFlags = 0 ) const;
        [[nodiscard]] VkCommandBufferSubmitInfo make_submit_info( uint32_t device_mask = 0 ) const;

    private:
        CommandPool& pool_ref_;
        DeviceSet const& device_ref_;

        VkCommandBufferLevel const buffer_level_{ VK_COMMAND_BUFFER_LEVEL_MAX_ENUM };
        VkCommandBuffer command_buffer_{ VK_NULL_HANDLE };

        size_t const buffer_index_{ UINT32_MAX };

        explicit CommandBuffer( CommandPool&, DeviceSet const&, VkCommandBufferLevel level, size_t index );

    };

}


#endif //!COMMANDBUFFER_H
