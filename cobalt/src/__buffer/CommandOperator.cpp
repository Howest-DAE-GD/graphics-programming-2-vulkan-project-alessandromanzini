#include <__buffer/CommandOperator.h>

#include <log.h>
#include <__buffer/Buffer.h>
#include <__image/Image.h>
#include <__meta/expect_size.h>
#include <__pipeline/Pipeline.h>
#include <__validation/result.h>


namespace cobalt
{
    CommandOperator::CommandOperator( VkCommandBuffer const command_buffer, VkCommandBufferBeginInfo const& begin_info )
        : command_buffer_{ command_buffer }
    {
        validation::throw_on_bad_result( vkBeginCommandBuffer( command_buffer_, &begin_info ),
                                         "Failed to begin recording command buffer!" );
        recording_ = true;
    }


    CommandOperator::~CommandOperator( ) noexcept
    {
        end_recording( );
    }


    CommandOperator::CommandOperator( CommandOperator&& other ) noexcept
        : command_buffer_{ other.command_buffer_ }
        , recording_{ std::exchange( other.recording_, false ) }
        , render_area_{ other.render_area_ }
        , viewport_{ other.viewport_ }
    {
        meta::expect_size<CommandOperator, 56u>( );
    }


    void CommandOperator::store_render_area( VkRect2D const& area )
    {
        render_area_ = area;
    }


    void CommandOperator::store_viewport( VkViewport const& viewport )
    {
        viewport_ = viewport;
    }


    void CommandOperator::end_recording( )
    {
        if ( recording_ )
        {
            validation::throw_on_bad_result( vkEndCommandBuffer( command_buffer_ ), "Failed to end recording command buffer!" );
            recording_ = false;
        }
    }


    void CommandOperator::begin_rendering( std::span<VkRenderingAttachmentInfo const> color_attachments,
                                           VkRenderingAttachmentInfo const* depth_attachment,
                                           std::optional<VkRect2D> const& render_area_override ) const
    {
        VkRenderingInfo const render_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = render_area_override.has_value( ) ? render_area_override.value( ) : render_area_,
            .layerCount = 1,
            .colorAttachmentCount = static_cast<uint32_t>( color_attachments.size( ) ),
            .pColorAttachments = color_attachments.data( ),
            .pDepthAttachment = depth_attachment
        };
        vkCmdBeginRendering( command_buffer_, &render_info );
    }


    void CommandOperator::end_rendering( ) const
    {
        vkCmdEndRendering( command_buffer_ );
    }


    void CommandOperator::insert_barrier( VkDependencyInfo const& dep_info ) const
    {
        vkCmdPipelineBarrier2( command_buffer_, &dep_info );
    }


    void CommandOperator::set_viewport( std::optional<VkViewport> const& viewport_override ) const
    {
        VkViewport const* viewport_ptr = viewport_override.has_value( ) ? &viewport_override.value( ) : &viewport_;
        vkCmdSetViewport( command_buffer_, 0, 1, viewport_ptr );
    }


    void CommandOperator::set_scissor( std::optional<VkRect2D> const& scissor_override ) const
    {
        VkRect2D const* rect_ptr = scissor_override.has_value( ) ? &scissor_override.value( ) : &render_area_;
        vkCmdSetScissor( command_buffer_, 0, 1, rect_ptr );
    }


    void CommandOperator::bind_pipeline( VkPipelineBindPoint const bind_point, Pipeline const& pipeline ) const
    {
        vkCmdBindPipeline( command_buffer_, bind_point, pipeline.handle( ) );
    }


    void CommandOperator::bind_descriptor_set(
        VkPipelineBindPoint const bind_point, Pipeline const& pipeline, VkDescriptorSet const desc_set ) const
    {
        vkCmdBindDescriptorSets( command_buffer_, bind_point, pipeline.layout( ).handle( ), 0, 1,
                                 &desc_set, 0, nullptr );
    }


    void CommandOperator::bind_pipeline_and_set(
        VkPipelineBindPoint const bind_point, Pipeline const& pipeline, VkDescriptorSet const desc_set ) const
    {
        bind_pipeline( bind_point, pipeline );
        bind_descriptor_set( bind_point, pipeline, desc_set );
    }


    void CommandOperator::bind_vertex_buffers( Buffer const& buffer, VkDeviceSize const offset ) const
    {
        VkBuffer const handle = buffer.handle( );
        vkCmdBindVertexBuffers( command_buffer_, 0, 1, &handle, &offset );
    }


    void CommandOperator::bind_index_buffer( Buffer const& buffer, VkDeviceSize const offset ) const
    {
        VkIndexType const index_type = to_index_type( buffer.content_type( ) );
        vkCmdBindIndexBuffer( command_buffer_, buffer.handle( ), offset, index_type );
    }


    void CommandOperator::push_constants( PipelineLayout const& layout, VkShaderStageFlags const stage_flags,
                                          uint32_t const offset, uint32_t const size, void const* data ) const
    {
        vkCmdPushConstants( command_buffer_, layout.handle( ), stage_flags, offset, size, data );
    }


    void CommandOperator::draw( int32_t const vertex_count, uint32_t const instance_count, uint32_t const vertex_offset,
                                uint32_t const instance_offset ) const
    {
        vkCmdDraw( command_buffer_, vertex_count, instance_count, vertex_offset, instance_offset );
    }


    void CommandOperator::draw_indexed( uint32_t const index_count, uint32_t const instance_count, uint32_t const index_offset,
                                        int32_t const vertex_offset, uint32_t const instance_offset ) const
    {
        vkCmdDrawIndexed( command_buffer_, index_count, instance_count, index_offset, vertex_offset, instance_offset );
    }


    void CommandOperator::copy_buffer_to_image( Buffer const& src, Image const& dst, VkBufferImageCopy const& region ) const
    {
        vkCmdCopyBufferToImage(
            command_buffer_,
            src.handle( ),
            dst.handle( ),
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
    }


    void CommandOperator::copy_buffer( Buffer const& src, Buffer const& dst ) const
    {
        VkBufferCopy const copy_region{
            .size = src.buffer_size( )
        };
        vkCmdCopyBuffer( command_buffer_, src.handle( ), dst.handle( ), 1, &copy_region );
    }

}
