#ifndef MEMORY_QUERIES_H
#define MEMORY_QUERIES_H

#include <vulkan/vulkan_core.h>


namespace cobalt_vk::query
{
    // +---------------------------+
    // | QUERIES                   |
    // +---------------------------+
    [[nodiscard]] uint32_t find_memory_type( VkPhysicalDevice physicalDevice, uint32_t typeFilter,
                                             VkMemoryPropertyFlags properties );

}


#endif //!MEMORY_QUERIES_H
