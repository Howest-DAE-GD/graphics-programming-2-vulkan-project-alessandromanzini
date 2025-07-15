#ifndef QUEUE_FAMILY_H
#define QUEUE_FAMILY_H

#include <vulkan/vulkan_core.h>

#include <optional>
#include <__context/InstanceBundle.h>


namespace cobalt::query
{
    // +---------------------------+
    // | STRUCTS                   |
    // +---------------------------+
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphics_family{};
        std::optional<uint32_t> present_family{};


        [[nodiscard]] bool is_suitable( ) const
        {
            return graphics_family.has_value( ) && present_family.has_value( );
        }

    };


    // +---------------------------+
    // | FUNCTIONS                 |
    // +---------------------------+
    [[nodiscard]] QueueFamilyIndices find_queue_families( VkPhysicalDevice physical_device, InstanceBundle const& instance );

}


#endif //!QUEUE_FAMILY_H
