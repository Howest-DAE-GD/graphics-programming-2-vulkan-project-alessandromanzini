#include <assets/Buffer.h>

#include <SingleTimeCommand.h>
#include <__query/device_queries.h>

#include <__validation/result.h>


namespace cobalt
{
    void Buffer::vk_create_buffer( VkDevice const device, VkPhysicalDevice const physical_device, const VkDeviceSize size, const VkBufferUsageFlags usage, VkMemoryPropertyFlags const properties,
        VkBuffer& buffer, VkDeviceMemory& buffer_memory )
    {
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size        = size;
        buffer_info.usage       = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        validation::throw_on_bad_result( vkCreateBuffer( device, &buffer_info, nullptr, &buffer ), "Failed to create buffer!" );

        // The first step of allocating memory for the buffer is to query its memory requirements.
        // The VkMemoryRequirements struct has three fields:
        // 1. size: The size of the required amount of memory in bytes, may differ from bufferInfo.size.
        // 2. alignment: The offset in bytes where the buffer begins in the allocated region of memory, depends on
        //    bufferInfo.usage and bufferInfo.flags
        // 3. memoryTypeBits: Bit field of the memory types that are suitable for the buffer.
        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements( device, buffer, &mem_requirements );

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize  = mem_requirements.size;
        alloc_info.memoryTypeIndex = query::find_memory_type( physical_device, mem_requirements.memoryTypeBits, properties );

        validation::throw_on_bad_result( vkAllocateMemory( device, &alloc_info, nullptr, &buffer_memory ), "Failed to allocate buffer memory!" );

        // If memory allocation was successful, then we can now associate this memory with the buffer.
        // Since this memory is allocated specifically for this vertex buffer, the offset is simply 0. If the offset is
        // non-zero, then it is required to be divisible by memRequirements.alignment.
        vkBindBufferMemory( device, buffer, buffer_memory, 0 );
    }


    void Buffer::vk_copy_buffer( VkDevice const device, VkCommandPool const command_pool, VkQueue const graphics_queue, VkBuffer const src_buffer, VkBuffer const dst_buffer, VkDeviceSize const size )
    {
        VkCommandBuffer const command_buffer{ cobalt::begin_single_time_commands( device, command_pool ) };

        VkBufferCopy copy_region{};
        copy_region.size = size;
        vkCmdCopyBuffer( command_buffer, src_buffer, dst_buffer, 1, &copy_region );

        end_single_time_commands( device, command_pool, command_buffer, graphics_queue );
    }


}
