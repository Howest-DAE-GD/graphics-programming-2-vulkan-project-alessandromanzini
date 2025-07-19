#include <__query/queue_family.h>

#include <vector>


namespace cobalt::query
{
    QueueFamilyIndices find_queue_families( VkPhysicalDevice const physical_device, InstanceBundle const& instance )
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
        uint32_t queue_family_count{ 0 };
        vkGetPhysicalDeviceQueueFamilyProperties( physical_device, &queue_family_count, nullptr );

        std::vector<VkQueueFamilyProperties> queue_families( queue_family_count );
        vkGetPhysicalDeviceQueueFamilyProperties( physical_device, &queue_family_count, queue_families.data( ) );

        // The VkQueueFamilyProperties struct contains some details about the queue family, including the type of operations
        // that are supported and the number of queues that can be created based on that family.
        for ( auto it{ queue_families.cbegin( ) }; it != queue_families.cend( ); ++it )
        {
            auto const index{ static_cast<uint32_t>( std::distance( queue_families.cbegin( ), it ) ) };
            if ( it->queueFlags & VK_QUEUE_GRAPHICS_BIT )
            {
                indices.graphics_family = index;
            }

            VkBool32 present_support{ false };
            vkGetPhysicalDeviceSurfaceSupportKHR( physical_device, static_cast<uint32_t>( index ), instance.surface( ),
                                                  &present_support );
            if ( present_support )
            {
                indices.present_family = index;
            }

            if ( indices.is_suitable( ) )
            {
                break;
            }
        }
        return indices;
    }
}
