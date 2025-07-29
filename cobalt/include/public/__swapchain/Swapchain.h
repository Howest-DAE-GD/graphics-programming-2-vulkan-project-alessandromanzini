#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <__memory/Resource.h>

#include <assets/Window.h>
#include <__image/Image.h>

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt
{
    class VkContext;
    class Framebuffer;
}

namespace cobalt
{
    struct SwapchainCreateInfo
    {
        uint32_t image_count{ 3 };
        VkPresentModeKHR present_mode{ VK_PRESENT_MODE_MAILBOX_KHR };
        VkSurfaceFormatKHR surface_format{ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    };


    class Swapchain final : public memory::Resource
    {
    public:
        explicit Swapchain( VkContext const& context, Window& window, SwapchainCreateInfo const& info );
        ~Swapchain( ) override;

        Swapchain( const Swapchain& )                = delete;
        Swapchain( Swapchain&& ) noexcept            = delete;
        Swapchain& operator=( const Swapchain& )     = delete;
        Swapchain& operator=( Swapchain&& ) noexcept = delete;

        [[nodiscard]] VkSwapchainKHR handle( ) const;
        [[nodiscard]] VkSwapchainKHR const* handle_ptr( ) const;

        [[nodiscard]] VkFormat image_format( ) const;
        [[nodiscard]] VkExtent2D extent( ) const;

        [[nodiscard]] std::vector<Image> const& images( ) const;

    private:
        VkContext const& context_ref_;
        Window& window_;

        SwapchainCreateInfo const create_info_;

        VkSwapchainKHR swapchain_{ VK_NULL_HANDLE };
        VkExtent2D swapchain_extent_{};
        VkFormat image_format_{};

        std::vector<Image> images_{};

        void init_swapchain( );
        void destroy_swapchain( );
        void recreate_swapchain( VkExtent2D extent );

    };

}


#endif //!SWAPCHAIN_H
