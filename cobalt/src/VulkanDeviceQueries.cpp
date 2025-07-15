#include "VulkanDeviceQueries.h"
#include <stdexcept>


namespace cobalt::query
{
    uint32_t find_memory_type( VkPhysicalDevice const physical_device, uint32_t const type_filter,
                               VkMemoryPropertyFlags const properties )
    {
        VkPhysicalDeviceMemoryProperties mem_properties;
        vkGetPhysicalDeviceMemoryProperties( physical_device, &mem_properties );
        for ( uint32_t i = 0; i < mem_properties.memoryTypeCount; i++ )
        {
            if ( type_filter & ( 1 << i ) && ( mem_properties.memoryTypes[i].propertyFlags & properties ) == properties )
            {
                return i;
            }
        }
        throw std::runtime_error( "Failed to find suitable memory type!" );
    }


    VkFormat find_supported_format( VkPhysicalDevice const physical_device, std::vector<VkFormat> const& candidates,
                                    VkImageTiling const tiling, VkFormatFeatureFlags const features )
    {
        for ( VkFormat const format : candidates )
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties( physical_device, format, &props );

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


    VkFormat find_depth_format( VkPhysicalDevice const physical_device )
    {
        return find_supported_format(
            physical_device,
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }


    bool has_stencil_component( VkFormat const format )
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

}
