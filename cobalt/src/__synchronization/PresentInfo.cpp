#include <__synchronization/PresentInfo.h>

#include <__render/Swapchain.h>
#include <__synchronization/Semaphore.h>


namespace cobalt::sync
{
    PresentInfo::PresentInfo( ) noexcept
    {
        present_info_ = VkPresentInfoKHR{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .pResults = nullptr,

            .waitSemaphoreCount = 0,
            .pWaitSemaphores = wait_semaphores_.data(),

            .swapchainCount = 0,
            .pSwapchains = swapchains_.data(),
            .pImageIndices = image_indices_.data()
        };
    }



    PresentInfo& PresentInfo::wait( Semaphore const& semaphore ) & noexcept
    {
        wait_semaphores_[present_info_.waitSemaphoreCount] = semaphore.handle( );
        ++present_info_.waitSemaphoreCount;
        return *this;
    }

    PresentInfo&& PresentInfo::wait( Semaphore const& semaphore ) && noexcept
    {
        return std::move( wait( semaphore ) );
    }


    PresentInfo& PresentInfo::present( Swapchain const& swapchain, uint32_t const image_index ) & noexcept
    {
        swapchains_[present_info_.swapchainCount] = swapchain.handle( );
        image_indices_[present_info_.swapchainCount] = image_index;
        ++present_info_.swapchainCount;
        return *this;
    }


    PresentInfo&& PresentInfo::present( Swapchain const& swapchain, uint32_t const image_index ) && noexcept
    {
        return std::move( present( swapchain, image_index ) );
    }


    VkPresentInfoKHR const& PresentInfo::info( ) const noexcept
    {
        return present_info_;
    }

}
