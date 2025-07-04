#ifndef QUEUE_FAMILY_H
#define QUEUE_FAMILY_H

#include <vulkan/vulkan_core.h>

#include <optional>


namespace cobalt_vk::query
{
    // +---------------------------+
    // | STRUCTS                   |
    // +---------------------------+
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily{};
        std::optional<uint32_t> presentFamily{};


        [[nodiscard]] bool is_suitable( ) const
        {
            return graphicsFamily.has_value( ) && presentFamily.has_value( );
        }

    };


    // +---------------------------+
    // | FUNCTIONS                 |
    // +---------------------------+
    [[nodiscard]] QueueFamilyIndices find_queue_families( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface );

}


#endif //!QUEUE_FAMILY_H
