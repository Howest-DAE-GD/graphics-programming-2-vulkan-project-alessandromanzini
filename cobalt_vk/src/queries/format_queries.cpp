#include <../../include/queries/format_queries.h>

#include <../../include/validation/dispatch.h>


namespace cobalt_vk::query
{
    // ReSharper disable once CppNotAllPathsReturnValue
    VkFormat find_supported_format( const VkPhysicalDevice physicalDevice,
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

        validation::throw_runtime_error( "Failed to find supported format!" );
        return {};
    }


    VkFormat find_depth_format( const VkPhysicalDevice physicalDevice )
    {
        return find_supported_format(
            physicalDevice,
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }


    bool has_stencil_component( const VkFormat format )
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

}
