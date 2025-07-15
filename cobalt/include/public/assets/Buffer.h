#ifndef BUFFER_H
#define BUFFER_H

#include <vulkan/vulkan_core.h>


namespace cobalt {
    class Buffer
    {
    public:
        static void vk_create_buffer( VkDevice device, VkPhysicalDevice physical_device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                               VkBuffer& buffer, VkDeviceMemory& buffer_memory );
        static void vk_copy_buffer( VkDevice device, VkCommandPool command_pool, VkQueue graphics_queue, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size );

    private:
        VkBuffer buffer_{ VK_NULL_HANDLE };

    };

}


#endif //!BUFFER_H
