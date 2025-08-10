#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <__memory/Resource.h>

#include <vulkan/vulkan_core.h>


namespace cobalt
{
    class DeviceSet;
}

namespace cobalt::sync
{
    /**
     * A semaphore is used to add order between queue operations. Queue operations refer to the work we submit to a
     * queue, either in a command buffer or from within a function. The queueing happens on the CPU, but the execution
     * and the waiting happen on the GPU.
     */
    class Semaphore final : public memory::Resource
    {
    public:
        explicit Semaphore( DeviceSet const&, VkSemaphoreCreateFlags create_flags = 0 ) noexcept;
        ~Semaphore( ) noexcept override;

        Semaphore( Semaphore&& ) noexcept;
        Semaphore( const Semaphore& )                = delete;
        Semaphore& operator=( const Semaphore& )     = delete;
        Semaphore& operator=( Semaphore&& ) noexcept = delete;

        [[nodiscard]] VkSemaphore handle( ) const noexcept;
        [[nodiscard]] VkSemaphoreSubmitInfo make_submit_info( VkPipelineStageFlags, uint32_t device_idx = 0 ) const noexcept;

    private:
        DeviceSet const& device_ref_;
        VkSemaphore semaphore_{ VK_NULL_HANDLE };

    };

}


#endif //!SEMAPHORE_H
