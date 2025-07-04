#include <assets/Buffer.h>

#include <ShaderModules.h>
#include <../../include/public/SingleTimeCommand.h>
#include <VulkanDeviceQueries.h>

#include <validation/result.h>

#include <stdexcept>


namespace cobalt_vk
{
    void Buffer::vk_create_buffer( const VkDevice device, VkPhysicalDevice physicalDevice, const VkDeviceSize size, const VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory )
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size        = size;
        bufferInfo.usage       = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        validation::throw_on_bad_result( vkCreateBuffer( device, &bufferInfo, nullptr, &buffer ), "Failed to create buffer!" );

        // The first step of allocating memory for the buffer is to query its memory requirements.
        // The VkMemoryRequirements struct has three fields:
        // 1. size: The size of the required amount of memory in bytes, may differ from bufferInfo.size.
        // 2. alignment: The offset in bytes where the buffer begins in the allocated region of memory, depends on
        //    bufferInfo.usage and bufferInfo.flags
        // 3. memoryTypeBits: Bit field of the memory types that are suitable for the buffer.
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements( device, buffer, &memRequirements );

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize  = memRequirements.size;
        allocInfo.memoryTypeIndex = cobalt_vk::query::find_memory_type( physicalDevice, memRequirements.memoryTypeBits, properties );

        validation::throw_on_bad_result( vkAllocateMemory( device, &allocInfo, nullptr, &bufferMemory ), "Failed to allocate buffer memory!" );

        // If memory allocation was successful, then we can now associate this memory with the buffer.
        // Since this memory is allocated specifically for this vertex buffer, the offset is simply 0. If the offset is
        // non-zero, then it is required to be divisible by memRequirements.alignment.
        vkBindBufferMemory( device, buffer, bufferMemory, 0 );
    }


    void Buffer::vk_copy_buffer( VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size )
    {
        VkCommandBuffer commandBuffer{ cobalt_vk::begin_single_time_commands( device, commandPool ) };

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );

        cobalt_vk::end_single_time_commands( device, commandPool, commandBuffer, graphicsQueue );
    }


}
