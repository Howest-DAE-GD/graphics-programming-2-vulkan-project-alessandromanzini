#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H

#include <__memory/Resource.h>

#include <__context/Window.h>
#include <__image/Image.h>
#include <__init/InitWizard.h>

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt
{
    namespace sync
    {
        class Semaphore;
    }
    class VkContext;
    class FrameBuffer;
    using SwapchainWizard = InitWizard<VkContext const&, Window&, struct SwapchainCreateInfo const&>::WithFeatures<>;
}

namespace cobalt
{
    struct SwapchainCreateInfo
    {
        uint32_t image_count{ 3 };
        VkPresentModeKHR present_mode{ VK_PRESENT_MODE_MAILBOX_KHR };
        VkSurfaceFormatKHR surface_format{ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
        bool create_depth_image{ true };
    };


    class Swapchain final : public memory::Resource
    {
    public:
        explicit Swapchain( SwapchainWizard );
        ~Swapchain( ) override;

        Swapchain( const Swapchain& )                = delete;
        Swapchain( Swapchain&& ) noexcept            = delete;
        Swapchain& operator=( const Swapchain& )     = delete;
        Swapchain& operator=( Swapchain&& ) noexcept = delete;

        [[nodiscard]] VkSwapchainKHR handle( ) const;
        [[nodiscard]] VkSwapchainKHR const* handle_ptr( ) const;

        [[nodiscard]] VkFormat image_format( ) const;
        [[nodiscard]] VkExtent2D extent( ) const;

        [[nodiscard]] uint32_t image_count( ) const;
        [[nodiscard]] Image& image_at( size_t index );
        [[nodiscard]] Image& depth_image( ) const;

        [[nodiscard]] uint32_t acquire_next_image( sync::Semaphore const& semaphore, uint64_t timeout = UINT64_MAX );

    private:
        VkContext const& context_ref_;
        Window& window_ref_;
        SwapchainCreateInfo const create_info_;

        VkSwapchainKHR swapchain_{ VK_NULL_HANDLE };
        VkExtent2D extent_{};
        VkFormat image_format_{};

        std::vector<Image> images_{};
        std::unique_ptr<Image> depth_image_ptr_{ nullptr };

        void init_swapchain( );
        void destroy_swapchain( );
        void recreate_swapchain( VkExtent2D extent );

    };

}


#endif //!SWAPCHAIN_H
