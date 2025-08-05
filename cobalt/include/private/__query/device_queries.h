#ifndef DEVICE_QUERIES_H
#define DEVICE_QUERIES_H

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt::query
{
    [[nodiscard]] uint32_t find_memory_type( VkPhysicalDevice, uint32_t type_filter, VkMemoryPropertyFlags properties );

    [[nodiscard]] VkFormat find_depth_format( VkPhysicalDevice );
    [[nodiscard]] VkFormat select_format( std::vector<VkFormat> const&, VkPhysicalDevice, VkImageTiling, VkFormatFeatureFlags );

    [[nodiscard]] bool has_stencil_component( VkFormat format );
}


#endif //!DEVICE_QUERIES_H
