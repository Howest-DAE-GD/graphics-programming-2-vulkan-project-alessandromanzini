#include <__synchronization/SubmitInfo.h>

#include <__buffer/CommandBuffer.h>
#include <__synchronization/Semaphore.h>


namespace cobalt::sync
{
    SubmitInfo::SubmitInfo( uint32_t const device_idx, VkSubmitFlags const flags ) noexcept
        : device_idx_{ device_idx }
    {
        submit_info_ = VkSubmitInfo2{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .pNext = nullptr,
            .flags = flags,

            .waitSemaphoreInfoCount = 0,
            .pWaitSemaphoreInfos = wait_semaphores_.data( ),

            .commandBufferInfoCount = 0,
            .pCommandBufferInfos = cmd_buffers_.data( ),

            .signalSemaphoreInfoCount = 0,
            .pSignalSemaphoreInfos = signal_semaphores_.data( ),
        };
    }


    SubmitInfo& SubmitInfo::wait( Semaphore const& semaphore, VkPipelineStageFlags const stage_mask ) & noexcept
    {
        wait_semaphores_[submit_info_.waitSemaphoreInfoCount] = semaphore.make_submit_info( stage_mask, device_idx_ );
        ++submit_info_.waitSemaphoreInfoCount;
        return *this;
    }


    SubmitInfo&& SubmitInfo::wait( Semaphore const& semaphore, VkPipelineStageFlags const stage_mask ) && noexcept
    {
        return std::move( wait( semaphore, stage_mask ) );
    }


    SubmitInfo& SubmitInfo::execute( CommandBuffer const& cmd_buffer ) & noexcept
    {
        cmd_buffers_[submit_info_.commandBufferInfoCount] = cmd_buffer.make_submit_info( device_idx_ );
        ++submit_info_.commandBufferInfoCount;
        return *this;
    }


    SubmitInfo&& SubmitInfo::execute( CommandBuffer const& cmd_buffer ) && noexcept
    {
        return std::move( execute( cmd_buffer ) );
    }


    SubmitInfo& SubmitInfo::signal( Semaphore const& semaphore, VkPipelineStageFlags const stage_mask ) & noexcept
    {
        signal_semaphores_[submit_info_.signalSemaphoreInfoCount] = semaphore.make_submit_info( stage_mask, device_idx_ );
        ++submit_info_.signalSemaphoreInfoCount;
        return *this;
    }


    SubmitInfo&& SubmitInfo::signal( Semaphore const& semaphore, VkPipelineStageFlags const stage_mask ) && noexcept
    {
        return std::move( signal( semaphore, stage_mask ) );
    }


    VkSubmitInfo2 const& SubmitInfo::info( ) const noexcept
    {
        return submit_info_;
    }

}
