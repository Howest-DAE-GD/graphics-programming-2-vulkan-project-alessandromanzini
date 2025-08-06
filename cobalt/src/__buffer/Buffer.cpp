#include <__buffer/Buffer.h>

#include <__context/DeviceSet.h>
#include <__meta/expect_size.h>
#include <__query/device_queries.h>
#include <__validation/result.h>

#include <cassert>


namespace cobalt
{

    Buffer::Buffer( DeviceSet const& device, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const
                    properties )
        : device_ref_{ device }
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
        , buffer_size_{ other.buffer_size_ }
        , memory_size_{ other.memory_size_ }
        , buffer_{ other.buffer_ }
        , buffer_memory_{ other.buffer_memory_ }
        , memory_map_ptr_{ other.memory_map_ptr_ }
    {
        meta::expect_size<Buffer, 56u>( );
        other.buffer_ = VK_NULL_HANDLE;
        other.buffer_memory_ = VK_NULL_HANDLE;
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


    VkMemoryRequirements Buffer::fetch_memory_requirements( ) const
    {
        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements( device_ref_.logical( ), buffer_, &mem_requirements );
        return mem_requirements;
    }


}
