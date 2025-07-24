#ifndef VKSWAPCHAINBUILDER_H
#define VKSWAPCHAINBUILDER_H

#include <vulkan/vulkan_core.h>

#include <vector>
#include <__query/swapchain_support.h>

#include "VkBuilder.h"


namespace cobalt
{
    class VkContext;
}

namespace cobalt
{
    struct VkSwapchainPopulateDetail
    {
        uint32_t image_count;
        VkExtent2D extent;
        VkPresentModeKHR present_mode_khr;
        VkSurfaceFormatKHR format_khr;
    };


    class VkSwapchainBuilder final : public builder::VkBuilder<VkSwapchainBuilder, decltype(&vkCreateSwapchainKHR)>
    {
    public:
        explicit VkSwapchainBuilder( VkContext const& context );

        VkSwapchainBuilder& set_image_buffering_aim( uint32_t );
        VkSwapchainBuilder& set_extent( VkExtent2D const& );
        VkSwapchainBuilder& set_extent( std::pair<int, int> const& );
        VkSwapchainBuilder& set_preferred_present_mode( VkPresentModeKHR );
        VkSwapchainBuilder& set_preferred_surface_format( VkSurfaceFormatKHR );

        VkSwapchainPopulateDetail populate_create_info( VkSwapchainCreateInfoKHR& create_info ) const;

        [[nodiscard]] query::SwapChainSupportDetails const& support_details( ) const;

    private:
        VkContext const& context_ref_;
        query::SwapChainSupportDetails const support_details_{};

        uint32_t image_buffering_aim_{ 1 };
        VkExtent2D extent_{};
        VkPresentModeKHR preferred_present_mode_{};
        VkSurfaceFormatKHR preferred_surface_format_{};

        [[nodiscard]] VkSwapchainPopulateDetail populate_detail( ) const;

        [[nodiscard]] uint32_t choose_image_buffering_aim( ) const;
        [[nodiscard]] VkSurfaceFormatKHR choose_surface_format( std::vector<VkSurfaceFormatKHR> const& available_formats ) const;
        [[nodiscard]] VkPresentModeKHR choose_present_mode( std::vector<VkPresentModeKHR> const& available_present_modes ) const;
        [[nodiscard]] VkExtent2D choose_extent( VkSurfaceCapabilitiesKHR const& capabilities ) const;

    };

}


#endif //!VKSWAPCHAINBUILDER_H
