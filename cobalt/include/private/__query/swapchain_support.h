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
    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats{};
        std::vector<VkPresentModeKHR> present_modes{};

        [[nodiscard]] bool is_adequate( ) const;
        explicit operator bool( ) const;
    };


    // +---------------------------+
    // | FUNCTIONS                 |
    // +---------------------------+
    [[nodiscard]] SwapchainSupportDetails check_swapchain_support( VkPhysicalDevice physical_device, InstanceBundle const& instance );

}


#endif //!SWAPCHAIN_SUPPORT_H
