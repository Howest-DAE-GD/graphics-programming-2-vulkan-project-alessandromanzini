#include <queries/memory_queries.h>

#include <validation/dispatch.h>


namespace cobalt_vk::query
{
    uint32_t find_memory_type( const VkPhysicalDevice physicalDevice, const uint32_t typeFilter,
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

        validation::throw_runtime_error( "Failed to find suitable memory type!" );
        return {};
    }

}
