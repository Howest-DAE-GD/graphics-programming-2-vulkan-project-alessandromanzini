#include "VulkanDeviceQueries.h"


using namespace engine::query;


QueueFamilyIndices engine::query::find_queue_families(
    const VkPhysicalDevice physicalDevice, // NOLINT(*-misplaced-const)
    const VkSurfaceKHR surface )           // NOLINT(*-misplaced-const)
{
    QueueFamilyIndices indices;

    // We can query basic device properties like the name, type and supported Vulkan version.
    // VkPhysicalDeviceProperties deviceProperties;
    // vkGetPhysicalDeviceProperties(device, &deviceProperties);

    // We can query the support for optional features like texture compression, 64bit floats and multi
    // viewport rendering.
    // VkPhysicalDeviceFeatures deviceFeatures;
    // vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    // Logic to find queue family indices to populate struct with.
    uint32_t queueFamilyCount{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, nullptr );

    std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
    vkGetPhysicalDeviceQueueFamilyProperties( physicalDevice, &queueFamilyCount, queueFamilies.data( ) );

    // The VkQueueFamilyProperties struct contains some details about the queue family, including the type of operations
    // that are supported and the number of queues that can be created based on that family.
    for ( auto it{ queueFamilies.cbegin( ) }; it != queueFamilies.cend( ); ++it )
    {
        const auto index{ std::distance( queueFamilies.cbegin( ), it ) };
        if ( it->queueFlags & VK_QUEUE_GRAPHICS_BIT )
        {
            indices.graphicsFamily = index;
        }

        VkBool32 presentSupport{ false };
        vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, index, surface, &presentSupport );
        if ( presentSupport )
        {
            indices.presentFamily = index;
        }

        if ( indices.is_suitable( ) )
        {
            break;
        }
    }
    return indices;
}


SwapChainSupportDetails engine::query::query_swap_chain_support(
    const VkPhysicalDevice physicalDevice, // NOLINT(*-misplaced-const)
    const VkSurfaceKHR surface )           // NOLINT(*-misplaced-const)
{
    // There are basically three kinds of properties we need to check:
    // 1. Basic surface capabilities ( min/max number of images in swap chain, min/max width and height of images )
    // 2. Surface formats( pixel format, color space )
    // 3. Available presentation modes
    SwapChainSupportDetails details{};

    // 1.
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( physicalDevice, surface, &details.capabilities );

    // 2.
    uint32_t formatCount{};
    vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount, nullptr );

    if ( formatCount != 0 )
    {
        details.formats.resize( formatCount );
        vkGetPhysicalDeviceSurfaceFormatsKHR( physicalDevice, surface, &formatCount, details.formats.data( ) );
    }

    // 3.
    uint32_t presentModeCount{};
    vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &presentModeCount, nullptr );

    if ( presentModeCount != 0 )
    {
        details.presentModes.resize( presentModeCount );
        vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, surface, &presentModeCount,
                                                   details.presentModes.data( ) );
    }

    return details;
}


uint32_t engine::query::find_memory_type( const VkPhysicalDevice physicalDevice, // NOLINT(*-misplaced-const)
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
