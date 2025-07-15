#ifndef SWAPCHAIN_SUPPORT_H
#define SWAPCHAIN_SUPPORT_H

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt
{
    class InstanceBundle;
}

namespace cobalt::query
{
    // +---------------------------+
    // | STRUCTS                   |
    // +---------------------------+
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> present_modes{};
    };


    // +---------------------------+
    // | FUNCTIONS                 |
    // +---------------------------+
    [[nodiscard]] SwapChainSupportDetails check_swap_chain_support( VkPhysicalDevice physical_device, InstanceBundle const& instance );

}


#endif //!SWAPCHAIN_SUPPORT_H
