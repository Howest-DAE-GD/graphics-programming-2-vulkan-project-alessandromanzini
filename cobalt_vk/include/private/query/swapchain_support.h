#ifndef SWAPCHAIN_SUPPORT_H
#define SWAPCHAIN_SUPPORT_H

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
    // | FUNCTIONS                 |
    // +---------------------------+
    [[nodiscard]] SwapChainSupportDetails query_swap_chain_support( VkPhysicalDevice physicalDevice, VkSurfaceKHR surface );

}


#endif //!SWAPCHAIN_SUPPORT_H
