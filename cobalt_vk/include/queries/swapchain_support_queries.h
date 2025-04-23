#ifndef SUPPORT_QUERIES_H
#define SUPPORT_QUERIES_H

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt_vk::query
{
    // +---------------------------+
    // | STRUCTS                   |
    // +---------------------------+
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> presentModes{};
    };


    // +---------------------------+
    // | QUERIES                   |
    // +---------------------------+
    [[nodiscard]] SwapChainSupportDetails query_swap_chain_support( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface );

}


#endif //!SUPPORT_QUERIES_H
