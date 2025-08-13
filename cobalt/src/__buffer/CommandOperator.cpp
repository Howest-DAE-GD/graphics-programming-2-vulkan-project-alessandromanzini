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
        , recording_{ other.recording_ }
    {
        meta::expect_size<CommandOperator, 16u>( );
        other.recording_ = false;
    }


    void CommandOperator::end_recording( )
    {
        if ( recording_ )
        {
            validation::throw_on_bad_result( vkEndCommandBuffer( command_buffer_ ), "Failed to end recording command buffer!" );
            recording_ = false;
        }
    }


    void CommandOperator::begin_rendering( VkRenderingInfo const& render_info ) const
    {
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


    void CommandOperator::set_viewport( VkViewport const& viewport ) const
    {
        vkCmdSetViewport( command_buffer_, 0, 1, &viewport );
    }


    void CommandOperator::set_viewport( std::span<VkViewport> const viewports ) const
    {
        vkCmdSetViewport( command_buffer_, 0, static_cast<uint32_t>( viewports.size( ) ), &viewports[0] );
    }


    void CommandOperator::set_scissor( VkRect2D const& scissor ) const
    {
        vkCmdSetScissor( command_buffer_, 0, 1, &scissor );
    }


    void CommandOperator::set_scissor( std::span<VkRect2D> const scissors ) const
    {
        vkCmdSetScissor( command_buffer_, 0, static_cast<uint32_t>( scissors.size( ) ), &scissors[0] );
    }


    void CommandOperator::bind_pipeline( VkPipelineBindPoint const bind_point, Pipeline const& pipeline ) const
    {
        vkCmdBindPipeline( command_buffer_, bind_point, pipeline.handle( ) );
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


    void CommandOperator::bind_descriptor_set( VkPipelineBindPoint const bind_point, Pipeline const& pipeline,
                                               VkDescriptorSet const desc_set ) const
    {
        vkCmdBindDescriptorSets( command_buffer_, bind_point, pipeline.layout( ).handle( ), 0, 1,
                                 &desc_set, 0, nullptr );
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
