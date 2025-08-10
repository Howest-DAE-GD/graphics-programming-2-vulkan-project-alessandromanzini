#ifndef RENDERSYNC_H
#define RENDERSYNC_H

#include <__synchronization/Fence.h>
#include <__synchronization/Semaphore.h>

#include <vector>


namespace cobalt
{
    class DeviceSet;
    class CommandPool;
    class CommandBuffer;
}

namespace cobalt::sync
{
    struct FrameSyncSet
    {
        CommandBuffer& cmd_buffer;
        Fence in_flight_fence;
        Semaphore acquire_semaphore;
    };

    class RenderSync final
    {
    public:
        explicit RenderSync( DeviceSet const&, CommandPool&, uint32_t max_frames_in_flight, uint32_t image_count );
        [[nodiscard]] FrameSyncSet const& frame_sync( uint32_t frame ) const;
        [[nodiscard]] Semaphore const& image_sync( uint32_t image_index ) const;

    private:
        std::vector<FrameSyncSet> sync_sets_{};
        std::vector<Semaphore> submit_semaphores_{};

    };

}


#endif //!RENDERSYNC_H
