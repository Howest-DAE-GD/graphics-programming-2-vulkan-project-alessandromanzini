#ifndef FORMAT_QUERIES_H
#define FORMAT_QUERIES_H

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt_vk::query
{
    // +---------------------------+
    // | QUERIES                   |
    // +---------------------------+
    [[nodiscard]] VkFormat find_depth_format( VkPhysicalDevice physicalDevice );

    [[nodiscard]] VkFormat find_supported_format( VkPhysicalDevice physicalDevice,
                                                  const std::vector<VkFormat>& candidates,
                                                  VkImageTiling tiling, VkFormatFeatureFlags features );

    [[nodiscard]] bool has_stencil_component( VkFormat format );

}


#endif //!FORMAT_QUERIES_H
