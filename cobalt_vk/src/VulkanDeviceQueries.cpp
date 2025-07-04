#include "VulkanDeviceQueries.h"
#include <stdexcept>


using namespace cobalt_vk::query;



uint32_t cobalt_vk::query::find_memory_type( const VkPhysicalDevice physicalDevice, // NOLINT(*-misplaced-const)
                                          const uint32_t typeFilter,
                                          const VkMemoryPropertyFlags properties )
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memProperties );

    for ( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ )
    {
        if ( typeFilter & ( 1 << i ) && ( memProperties.memoryTypes[i].propertyFlags & properties ) == properties )
        {
            return i;
        }
    }

    throw std::runtime_error( "Failed to find suitable memory type!" );
}


VkFormat cobalt_vk::query::find_supported_format( const VkPhysicalDevice physicalDevice,
                                               const std::vector<VkFormat>& candidates,
                                               const VkImageTiling tiling,
                                               const VkFormatFeatureFlags features )
{
    for ( const VkFormat format : candidates )
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties( physicalDevice, format, &props );

        if ( tiling == VK_IMAGE_TILING_LINEAR && ( props.linearTilingFeatures & features ) == features )
        {
            return format;
        }
        if ( tiling == VK_IMAGE_TILING_OPTIMAL && ( props.optimalTilingFeatures & features ) == features )
        {
            return format;
        }
    }

    throw std::runtime_error( "Failed to find supported format!" );
}


VkFormat cobalt_vk::query::find_depth_format( const VkPhysicalDevice physicalDevice )
{
    return find_supported_format(
        physicalDevice,
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}


bool cobalt_vk::query::has_stencil_component( const VkFormat format ) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}
