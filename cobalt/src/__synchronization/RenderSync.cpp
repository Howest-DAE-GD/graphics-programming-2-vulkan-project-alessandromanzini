#include <__synchronization/RenderSync.h>

#include <__buffer/CommandPool.h>
#include <__context/DeviceSet.h>


namespace cobalt::sync
{
    RenderSync::RenderSync( DeviceSet const& device, CommandPool& cmd_pool, uint32_t const max_frames_in_flight,
                            uint32_t const image_count )
    {
        for ( size_t i{}; i < max_frames_in_flight; i++ )
        {
            sync_sets_.emplace_back(
                cmd_pool.acquire( VK_COMMAND_BUFFER_LEVEL_PRIMARY ),
                Fence{ device, VK_FENCE_CREATE_SIGNALED_BIT },
                Semaphore{ device } );
        }
        for ( size_t i{}; i < image_count; i++ )
        {
            submit_semaphores_.emplace_back( device );
        }
    }


    FrameSyncSet const& RenderSync::frame_sync( uint32_t const frame ) const
    {
        return sync_sets_[frame % sync_sets_.size( )];
    }


    Semaphore const& RenderSync::image_sync( uint32_t const image_index ) const
    {
        return submit_semaphores_[image_index % submit_semaphores_.size( )];
    }

}
