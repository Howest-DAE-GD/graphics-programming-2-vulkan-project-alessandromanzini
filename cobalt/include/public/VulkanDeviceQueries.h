#ifndef VULKANDEVICEQUERIES_H
#define VULKANDEVICEQUERIES_H

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>


namespace cobalt::query
{
    // +---------------------------+
    // | MEMORY                    |
    // +---------------------------+
    [[nodiscard]] uint32_t find_memory_type( VkPhysicalDevice physical_device, uint32_t type_filter,
                                             VkMemoryPropertyFlags properties );

    // +---------------------------+
    // | FORMAT                    |
    // +---------------------------+
    [[nodiscard]] VkFormat find_supported_format( VkPhysicalDevice physical_device, std::vector<VkFormat> const& candidates,
                                                  VkImageTiling tiling, VkFormatFeatureFlags features );
    [[nodiscard]] VkFormat find_depth_format( VkPhysicalDevice physical_device );
    [[nodiscard]] bool has_stencil_component( VkFormat format );
}

#endif //VULKANDEVICEQUERIES_H
