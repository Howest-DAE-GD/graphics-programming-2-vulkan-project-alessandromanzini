#include <__buffer/Buffer.h>

#include <__buffer/CommandPool.h>
#include <__context/DeviceSet.h>
#include <__image/Image.h>
#include <__meta/expect_size.h>
#include <__query/device_queries.h>
#include <__validation/result.h>

#include <cassert>


namespace cobalt
{
    // +---------------------------+
    // | BUFFER                    |
    // +---------------------------+
    Buffer::Buffer( DeviceSet const& device, VkDeviceSize const size, VkBufferUsageFlags const usage,
                    VkMemoryPropertyFlags const properties, buffer::BufferContentType const content_type )
        : device_ref_{ device }
        , content_type_{ content_type }
        , buffer_size_{ size }
    {
        VkBufferCreateInfo const buffer_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = buffer_size_,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        validation::throw_on_bad_result(
            vkCreateBuffer( device_ref_.logical( ), &buffer_info, nullptr, &buffer_ ),
            "Failed to create buffer!" );

        // The first step of allocating memory for the buffer is to query its memory requirements.
        // The VkMemoryRequirements struct has three fields:
        // 1. size: The size of the required amount of memory in bytes, may differ from bufferInfo.size.
        // 2. alignment: The offset in bytes where the buffer begins in the allocated region of memory, depends on
        //    bufferInfo.usage and bufferInfo.flags
        // 3. memoryTypeBits: Bit field of the memory types that are suitable for the buffer.
        VkMemoryRequirements const mem_requirements = fetch_memory_requirements( );
        VkMemoryAllocateInfo const alloc_info{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = ( memory_size_ = mem_requirements.size ),
            .memoryTypeIndex = query::find_memory_type( device_ref_.physical( ), mem_requirements.memoryTypeBits, properties )
        };

        validation::throw_on_bad_result(
            vkAllocateMemory( device_ref_.logical( ), &alloc_info, nullptr, &buffer_memory_ ),
            "Failed to allocate buffer memory!" );

        // If memory allocation was successful, then we can now associate this memory with the buffer.
        // Since this memory is allocated specifically for this vertex buffer, the offset is simply 0. If the offset is
        // non-zero, then it is required to be divisible by memRequirements.alignment.
        vkBindBufferMemory( device_ref_.logical( ), buffer_, buffer_memory_, 0 );
    }


    Buffer::~Buffer( ) noexcept
    {
        // 1. Unmap the GPU memory
        if ( memory_map_ptr_ )
        {
            unmap_memory( );
        }

        // 2. Free the GPU buffer memory
        if ( buffer_memory_ != VK_NULL_HANDLE )
        {
            vkFreeMemory( device_ref_.logical( ), buffer_memory_, nullptr );
            buffer_memory_ = VK_NULL_HANDLE;
        }

        // 3. Destroy the buffer
        if ( buffer_ != VK_NULL_HANDLE )
        {
            vkDestroyBuffer( device_ref_.logical( ), buffer_, nullptr );
            buffer_ = VK_NULL_HANDLE;
        }
    }


    Buffer::Buffer( Buffer&& other ) noexcept
        : device_ref_{ other.device_ref_ }
        , content_type_{ other.content_type_ }
        , buffer_size_{ other.buffer_size_ }
        , memory_size_{ other.memory_size_ }
        , buffer_{ other.buffer_ }
        , buffer_memory_{ other.buffer_memory_ }
        , memory_map_ptr_{ other.memory_map_ptr_ }
    {
        meta::expect_size<Buffer, 64u>( );
        other.buffer_         = VK_NULL_HANDLE;
        other.buffer_memory_  = VK_NULL_HANDLE;
        other.memory_map_ptr_ = nullptr;
    }


    VkBuffer Buffer::handle( ) const
    {
        return buffer_;
    }


    VkDeviceMemory Buffer::memory( ) const
    {
        return buffer_memory_;
    }


    buffer::BufferContentType Buffer::content_type( ) const
    {
        return content_type_;
    }


    VkDeviceSize Buffer::buffer_size( ) const
    {
        return buffer_size_;
    }


    VkDeviceSize Buffer::memory_size( ) const
    {
        return memory_size_;
    }


    void* Buffer::data( ) const
    {
        assert( memory_map_ptr_ != nullptr && "Buffer::data: call map memory before getting data." );
        return memory_map_ptr_;
    }


    void Buffer::map_memory( VkDeviceSize const offset, VkMemoryMapFlags const flags )
    {
        vkMapMemory( device_ref_.logical( ), buffer_memory_, offset, memory_size_, flags, &memory_map_ptr_ );
    }


    void Buffer::unmap_memory( )
    {
        vkUnmapMemory( device_ref_.logical( ), buffer_memory_ );
        memory_map_ptr_ = nullptr;
    }


    void Buffer::copy_to( Buffer const& dst, CommandPool& cmd_pool ) const
    {
        auto const& cmd_buffer = cmd_pool.acquire( VK_COMMAND_BUFFER_LEVEL_PRIMARY );
        cmd_buffer.reset( 0 );

        auto buffer_op = cmd_buffer.command_operator( 0 );
        buffer_op.copy_buffer( *this, dst );
        buffer_op.end_recording( );

        device_ref_.graphics_queue( ).submit_and_wait( sync::SubmitInfo{ device_ref_.device_index( ) }.execute( cmd_buffer ) );
        cmd_buffer.unlock( );
    }


    void Buffer::copy_to( Image const& dst, CommandPool& cmd_pool ) const
    {
        auto const& cmd_buffer = cmd_pool.acquire( VK_COMMAND_BUFFER_LEVEL_PRIMARY );
        cmd_buffer.reset( 0 );

        auto buffer_op = cmd_buffer.command_operator( 0 );
        buffer_op.copy_buffer_to_image(
            *this, dst, VkBufferImageCopy{
                .bufferOffset = 0,
                .bufferRowLength = 0,
                .bufferImageHeight = 0,

                .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .imageSubresource.mipLevel = 0,
                .imageSubresource.baseArrayLayer = 0,
                .imageSubresource.layerCount = 1,

                .imageOffset = { 0, 0, 0 },
                .imageExtent = { dst.extent( ).width, dst.extent( ).height, 1 }
            } );
        buffer_op.end_recording( );

        device_ref_.graphics_queue( ).submit_and_wait( sync::SubmitInfo{ device_ref_.device_index( ) }.execute( cmd_buffer ) );
        cmd_buffer.unlock( );
    }


    VkMemoryRequirements Buffer::fetch_memory_requirements( ) const
    {
        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements( device_ref_.logical( ), buffer_, &mem_requirements );
        return mem_requirements;
    }


    // +---------------------------+
    // | FACTORY FUNCTIONS         |
    // +---------------------------+
    namespace buffer
    {
        Buffer internal::allocate_data_buffer( DeviceSet const& device, CommandPool& cmd_pool, void const* data,
                                               VkDeviceSize const size, VkBufferUsageFlags const main_usage_bit,
                                               BufferContentType const content_type )
        {
            Buffer staging_buffer = make_staging_buffer( device, size );

            staging_buffer.map_memory( );
            memcpy( staging_buffer.data( ), data, staging_buffer.memory_size( ) );
            staging_buffer.unmap_memory( );

            Buffer data_buffer{
                device, size,
                VK_BUFFER_USAGE_TRANSFER_DST_BIT | main_usage_bit,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, content_type
            };
            staging_buffer.copy_to( data_buffer, cmd_pool );
            return data_buffer;
        }


        Buffer make_staging_buffer( DeviceSet const& device, VkDeviceSize const size )
        {
            return Buffer{
                device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            };
        }


        Buffer make_uniform_buffer( DeviceSet const& device, VkDeviceSize const size )
        {
            Buffer uniform_buffer{
                device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                BufferContentType::UNIFORM
            };

            // The buffer stays mapped to this pointer for the application's whole lifetime. This technique is called
            // "persistent mapping" and works on all Vulkan implementations.
            uniform_buffer.map_memory( );

            return uniform_buffer;
        }

    }


}
