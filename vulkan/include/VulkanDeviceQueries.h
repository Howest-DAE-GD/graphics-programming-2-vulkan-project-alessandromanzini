#ifndef VULKANDEVICEQUERIES_H
#define VULKANDEVICEQUERIES_H

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>


namespace engine::query
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

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> presentModes{};
    };

    // +---------------------------+
    // | FUNCTIONS                 |
    // +---------------------------+
    QueueFamilyIndices find_queue_families( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface );
    SwapChainSupportDetails query_swap_chain_support( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface );

    // +---------------------------+
    // | MEMORY                    |
    // +---------------------------+
    uint32_t find_memory_type( VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties );
}

#endif //VULKANDEVICEQUERIES_H
