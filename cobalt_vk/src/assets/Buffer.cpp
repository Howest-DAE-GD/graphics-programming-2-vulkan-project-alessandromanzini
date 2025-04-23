#include <assets/Buffer.h>

#include <ShaderModules.h>
#include <SingleTimeCommand.h>

#include <validation/dispatch.h>
#include <validation/result.h>

#include <queries/memory_queries.h>


namespace cobalt_vk
{
    Buffer::Buffer( const VkDevice device, const VkPhysicalDevice physicalDevice, const VkDeviceSize size,
                    const VkBufferUsageFlags usage,
                    const VkMemoryPropertyFlags properties )
        : size_{ size }
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size        = size;
        bufferInfo.usage       = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        validation::throw_on_bad_result( vkCreateBuffer( device, &bufferInfo, nullptr, &buffer_ ), "Failed to create buffer!" );

        // The first step of allocating memory for the buffer is to query its memory requirements.
        // The VkMemoryRequirements struct has three fields:
        // 1. size: The size of the required amount of memory in bytes, may differ from bufferInfo.size.
        // 2. alignment: The offset in bytes where the buffer begins in the allocated region of memory, depends on
        //    bufferInfo.usage and bufferInfo.flags
        // 3. memoryTypeBits: Bit field of the memory types that are suitable for the buffer.
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements( device, buffer_, &memRequirements );

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize  = memRequirements.size;
        allocInfo.memoryTypeIndex = query::find_memory_type( physicalDevice, memRequirements.memoryTypeBits, properties );


        validation::throw_on_bad_result( vkAllocateMemory( device, &allocInfo, nullptr, &buffer_memory_ ),
                                         "Failed to allocate buffer memory!" );

        // If memory allocation was successful, then we can now associate this memory with the buffer.
        // Since this memory is allocated specifically for this vertex buffer, the offset is simply 0. If the offset is
        // non-zero, then it is required to be divisible by memRequirements.alignment.
        vkBindBufferMemory( device, buffer_, buffer_memory_, 0 );
    }


    Buffer::Buffer( Buffer&& other ) noexcept
        : buffer_{ other.buffer_ }
        , buffer_memory_{ other.buffer_memory_ }
        , size_{ other.size_ } { }


    Buffer& Buffer::operator=( Buffer&& other ) noexcept
    {
        buffer_        = other.buffer_;
        buffer_memory_ = other.buffer_memory_;
        size_          = other.size_;

        return *this;
    }


    void Buffer::map_memory( const VkDevice device, const VkDeviceSize offset, const VkDeviceSize size,
                             const VkMemoryMapFlags flags, void** data ) const
    {
        vkMapMemory( device, buffer_memory_, offset, size, flags, data );
    }


    void Buffer::unmap_memory( const VkDevice device ) const
    {
        vkUnmapMemory( device, buffer_memory_ );
    }


    void Buffer::release( const VkDevice device )
    {
        if ( buffer_ != VK_NULL_HANDLE )
        {
            vkDestroyBuffer( device, buffer_, nullptr );
        }
        if ( buffer_memory_ != VK_NULL_HANDLE )
        {
            vkFreeMemory( device, buffer_memory_, nullptr );
        }
    }


    void Buffer::copy_buffer( const VkDevice device, const VkCommandPool commandPool, const VkQueue graphicsQueue,
                              const Buffer* src, const Buffer* dst )
    {
        const VkCommandBuffer commandBuffer = engine::begin_single_time_commands( device, commandPool );

        VkBufferCopy copyRegion{};
        copyRegion.size = std::min( src->size_, dst->size_ );
        vkCmdCopyBuffer( commandBuffer, src->buffer_, dst->buffer_, 1, &copyRegion );

        engine::end_single_time_commands( device, commandPool, commandBuffer, graphicsQueue );
    }


}
