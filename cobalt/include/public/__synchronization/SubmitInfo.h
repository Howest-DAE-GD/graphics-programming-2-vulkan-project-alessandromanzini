#ifndef SUBMITINFO_H
#define SUBMITINFO_H

#include <array>
#include <cstdint>
#include <vulkan/vulkan_core.h>


namespace cobalt
{
    namespace sync
    {
        class Semaphore;
    }
    class DeviceSet;
    class CommandBuffer;
}

namespace cobalt::sync
{
    class SubmitInfo final
    {
    public:
        static constexpr size_t MAX_CHAINING_PER_ITEM{ 8 };

        explicit SubmitInfo( uint32_t device_idx, VkSubmitFlags flags = 0 ) noexcept;

        SubmitInfo& wait( Semaphore const&, VkPipelineStageFlags stage_mask ) & noexcept;
        SubmitInfo&& wait( Semaphore const&, VkPipelineStageFlags stage_mask ) && noexcept;

        SubmitInfo& execute( CommandBuffer const& ) & noexcept;
        SubmitInfo&& execute( CommandBuffer const& ) && noexcept;

        SubmitInfo& signal( Semaphore const&, VkPipelineStageFlags stage_mask ) & noexcept;
        SubmitInfo&& signal( Semaphore const&, VkPipelineStageFlags stage_mask ) && noexcept;

        [[nodiscard]] VkSubmitInfo2 const& info( ) const noexcept;

    private:
        uint32_t const device_idx_;

        VkSubmitInfo2 submit_info_{};

        std::array<VkSemaphoreSubmitInfo, MAX_CHAINING_PER_ITEM> wait_semaphores_{};
        std::array<VkSemaphoreSubmitInfo, MAX_CHAINING_PER_ITEM> signal_semaphores_{};
        std::array<VkCommandBufferSubmitInfo, MAX_CHAINING_PER_ITEM> cmd_buffers_{};

    };

}


#endif //!SUBMITINFO_H
