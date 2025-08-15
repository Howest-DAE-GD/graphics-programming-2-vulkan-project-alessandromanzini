#include <__render/Renderer.h>

#include <__buffer/CommandPool.h>
#include <__context/DeviceSet.h>
#include <__render/Swapchain.h>


namespace cobalt
{
    Renderer::Renderer( RendererCreateInfo const& create_info )
        : device_ref_{ *create_info.device }
        , swapchain_ref_{ *create_info.swapchain }
        , render_sync_{
            *create_info.device, *create_info.cmd_pool, create_info.max_frames_in_flight, create_info.swapchain->image_count( )
        }
        , max_frames_in_flight_{ create_info.max_frames_in_flight } { }


    void Renderer::set_record_command_buffer_fn( std::function<record_command_buffer_sig_t> record_fn ) noexcept
    {
        record_command_buffer_fn_ = std::move( record_fn );
    }


    void Renderer::set_update_uniform_buffer_fn( std::function<update_uniform_buffer_sig_t> update_fn ) noexcept
    {
        update_uniform_buffer_fn_ = std::move( update_fn );
    }


    VkResult Renderer::render( ) const
    {
        auto const& [cmd_buffer, in_flight_fence, acquire_semaphore] =
                render_sync_.frame_sync( static_cast<uint32_t>( current_frame_ ) );

        // 1. Wait for the previous frame to finish. We wait for the fence.
        in_flight_fence.wait( );

        // 2. Acquire an image from the swapchain.
        uint32_t const image_index = swapchain_ref_.acquire_next_image( acquire_semaphore );

        // If the image index is UINT32_MAX, it means that the swapchain could not acquire an image.
        if ( image_index == UINT32_MAX )
        {
            return VK_ERROR_UNKNOWN;
        }

        // After waiting, we need to manually reset the fence to the unsignaled state with the vkResetFences call.
        // We reset the fence only if there's work to do, which is why we're doing it after the acquire image check. DEADLOCK warning!
        in_flight_fence.reset( );

        // 3. Record a command buffer which draws the scene onto that image.
        if ( record_command_buffer_fn_ )
        {
            record_command_buffer_fn_( cmd_buffer, swapchain_ref_, image_index, static_cast<uint32_t>( current_frame_ ) );
        }
        if ( update_uniform_buffer_fn_ )
        {
            update_uniform_buffer_fn_( static_cast<uint32_t>( current_frame_ ) );
        }

        // 4. We need to submit the recorded command buffer to the graphics queue before submitting the image to the swapchain.
        auto const& submit_semaphore = render_sync_.image_sync( image_index );
        device_ref_.graphics_queue( ).submit(
            sync::SubmitInfo{ device_ref_.device_index( ) }
            .wait( acquire_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR )
            .execute( cmd_buffer )
            .signal( submit_semaphore, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR ),

            // This fence will be signaled when the command buffer finishes execution, allowing us to know when it is safe for it
            // to be reused.
            &in_flight_fence );

        // Switch to next frame for the next render call.
        current_frame_ = ( current_frame_ + 1 ) % max_frames_in_flight_;

        // 5. Present the swapchain image to the queue after the signaled semaphore.
        return device_ref_.graphics_queue( ).present(
            sync::PresentInfo{}.wait( submit_semaphore ).present( swapchain_ref_, image_index ) );
    }

}
