#include <__buffer/CommandOperator.h>

#include <log.h>
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
        if ( recording_ )
        {
            validation::throw_on_bad_result( vkEndCommandBuffer( command_buffer_ ), "Failed to end recording command buffer!" );
        }
    }


    CommandOperator::CommandOperator( CommandOperator&& other ) noexcept
        : command_buffer_{ other.command_buffer_ }
        , recording_{ other.recording_ }
    {
        other.recording_ = false;
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
        vkCmdSetViewport( command_buffer_, 0, viewports.size( ), &viewports[0] );
    }


    void CommandOperator::set_scissor( VkRect2D const& scissor ) const
    {
        vkCmdSetScissor( command_buffer_, 0, 1, &scissor );
    }


    void CommandOperator::set_scissor( std::span<VkRect2D> const scissors ) const
    {
        vkCmdSetScissor( command_buffer_, 0, scissors.size( ), &scissors[0] );
    }


    void CommandOperator::begin_rendering( VkRenderingInfo const& render_info ) const
    {
        vkCmdBeginRendering( command_buffer_, &render_info );
    }


    void CommandOperator::end_rendering( ) const
    {
        vkCmdEndRendering( command_buffer_ );
    }


    void CommandOperator::draw_indexed( uint32_t const index_count, uint32_t const instance_count, uint32_t const first_index,
                                        int32_t const vertex_offset, uint32_t const first_instance ) const
    {
        vkCmdDrawIndexed( command_buffer_, index_count, instance_count, first_index, vertex_offset, first_instance );
    }


    void CommandOperator::bind_pipeline( VkPipelineBindPoint const bind_point, Pipeline const& pipeline ) const
    {
        vkCmdBindPipeline( command_buffer_, bind_point, pipeline.handle( ) );
    }


    void CommandOperator::bind_vertex_buffers( VkBuffer const buffer, VkDeviceSize const offset ) const
    {
        vkCmdBindVertexBuffers( command_buffer_, 0, 1, &buffer, &offset );
    }


    void CommandOperator::bind_vertex_buffers( std::span<VkBuffer> const buffers, std::span<VkDeviceSize> const offsets ) const
    {
        if ( buffers.size( ) != offsets.size( ) )
        {
            log::logerr<CommandOperator>( "bind_vertex_buffers", "buffers and offsets must be the same size!" );
            return;
        }
        vkCmdBindVertexBuffers( command_buffer_, 0, 1, &buffers[0], &offsets[0] );
    }


    void CommandOperator::bind_index_buffer( VkBuffer const buffer, VkDeviceSize const offset, VkIndexType const type ) const
    {
        vkCmdBindIndexBuffer( command_buffer_, buffer, offset, type );
    }


    void CommandOperator::bind_descriptor_set( VkPipelineBindPoint const bind_point, Pipeline const& pipeline,
                                                VkDescriptorSet const desc_set ) const
    {
        vkCmdBindDescriptorSets( command_buffer_, bind_point, pipeline.layout( ), 0, 1,
                                 &desc_set, 0, nullptr );
    }

}
