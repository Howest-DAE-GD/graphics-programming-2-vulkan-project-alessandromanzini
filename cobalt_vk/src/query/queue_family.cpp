#include <query/queue_family.h>

#include <vector>


namespace cobalt_vk::query
{
    QueueFamilyIndices find_queue_families( const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface )
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
            const auto index{ static_cast<uint32_t>( std::distance( queueFamilies.cbegin( ), it ) ) };
            if ( it->queueFlags & VK_QUEUE_GRAPHICS_BIT )
            {
                indices.graphicsFamily = index;
            }

            VkBool32 presentSupport{ false };
            vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, static_cast<uint32_t>( index ), surface, &presentSupport );
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

}
