#ifndef PRESENTINFO_H
#define PRESENTINFO_H

#include <array>
#include <vulkan/vulkan_core.h>


namespace cobalt
{
    namespace sync
    {
        class Semaphore;
    }
    class Swapchain;
}

namespace cobalt::sync
{
    class PresentInfo final
    {
    public:
        static constexpr size_t MAX_CHAINING_PER_ITEM{ 8 };

        explicit PresentInfo( ) noexcept;

        PresentInfo& wait( Semaphore const& ) & noexcept;
        PresentInfo&& wait( Semaphore const& ) && noexcept;

        PresentInfo& present( Swapchain const&, uint32_t image_index ) & noexcept;
        PresentInfo&& present( Swapchain const&, uint32_t image_index ) && noexcept;

        [[nodiscard]] VkPresentInfoKHR const& info( ) const noexcept;

    private:
        VkPresentInfoKHR present_info_{};

        std::array<VkSemaphore, MAX_CHAINING_PER_ITEM> wait_semaphores_{};
        std::array<VkSwapchainKHR, MAX_CHAINING_PER_ITEM> swapchains_{};
        std::array<uint32_t, MAX_CHAINING_PER_ITEM> image_indices_{};

    };

}


#endif //!PRESENTINFO_H
