#ifndef BUFFER_H
#define BUFFER_H

#include <vulkan/vulkan_core.h>


namespace cobalt_vk {
    class Buffer
    {
    public:
        static void vk_create_buffer( VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                               VkBuffer& buffer, VkDeviceMemory& bufferMemory );
        static void vk_copy_buffer( VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size );

    private:
        VkBuffer buffer_{ VK_NULL_HANDLE };

    };

}


#endif //!BUFFER_H
