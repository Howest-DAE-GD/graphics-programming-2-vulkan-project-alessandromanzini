#include <__buffer/CommandBuffer.h>

#include <__buffer/CommandPool.h>
#include <__context/DeviceSet.h>
#include <__meta/expect_size.h>
#include <__pipeline/Pipeline.h>
#include <__validation/result.h>


namespace cobalt
{
    CommandBuffer::~CommandBuffer( ) noexcept
    {
        if ( command_buffer_ != VK_NULL_HANDLE )
        {
            vkFreeCommandBuffers( device_ref_.logical( ), pool_ref_.handle( ), 1, &command_buffer_ );
            command_buffer_ = VK_NULL_HANDLE;
        }
    }


    CommandBuffer::CommandBuffer( CommandBuffer&& other ) noexcept
        : pool_ref_{ other.pool_ref_ }
        , device_ref_{ other.device_ref_ }
        , buffer_level_{ other.buffer_level_ }
        , command_buffer_{ other.command_buffer_ }
        , buffer_index_{ other.buffer_index_ }
    {
        meta::expect_size<CommandBuffer, 48u>( );
        other.command_buffer_ = VK_NULL_HANDLE;
    }


    CommandBuffer::CommandBuffer( CommandPool& pool, DeviceSet const& device, VkCommandBufferLevel const level,
                                  size_t const index )
        : pool_ref_{ pool }
        , device_ref_{ device }
        , buffer_level_{ level }
        , buffer_index_{ index }
    {
        // The level parameter specifies if the allocated command buffers are primary or secondary command buffers.
        // - VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other
        // command buffers.
        // - VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers
        VkCommandBufferAllocateInfo const alloc_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = pool_ref_.handle( ),
            .level = buffer_level_,
            .commandBufferCount = 1
        };

        validation::throw_on_bad_result(
            vkAllocateCommandBuffers( device_ref_.logical( ), &alloc_info, &command_buffer_ ),
            "Failed to allocate command buffers!" );
    }


    void CommandBuffer::unlock( ) const noexcept
    {
        pool_ref_.release( buffer_index_ );
    }


    void CommandBuffer::reset( VkCommandBufferResetFlags const flags ) const
    {
        vkResetCommandBuffer( command_buffer_, flags );
    }


    CommandOperator CommandBuffer::command_operator( VkCommandBufferUsageFlags const flags ) const
    {
        // The flags parameter specifies how we're going to use the command buffer. The following values are available:
        // - VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it
        // once.
        // - VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely
        // within a single render pass.
        // - VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already
        // pending execution.
        return {
            command_buffer_,
            VkCommandBufferBeginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = flags,
                .pInheritanceInfo = nullptr
            }
        };
    }


    VkCommandBufferSubmitInfo CommandBuffer::make_submit_info( uint32_t const device_mask ) const
    {
        return {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
            .commandBuffer = command_buffer_,
            .deviceMask = device_mask
        };
    }

}
