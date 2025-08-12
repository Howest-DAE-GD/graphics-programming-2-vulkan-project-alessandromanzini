#ifndef COMMANDOPERATOR_H
#define COMMANDOPERATOR_H

#include <vulkan/vulkan_core.h>

#include <span>


namespace cobalt
{
    class Pipeline;
    class Image;
    class Buffer;
}

namespace cobalt
{
    class CommandOperator final
    {
    public:
        CommandOperator( VkCommandBuffer command_buffer, VkCommandBufferBeginInfo const& );
        ~CommandOperator( ) noexcept;

        CommandOperator( CommandOperator&& ) noexcept;
        CommandOperator( const CommandOperator& )                = delete;
        CommandOperator& operator=( const CommandOperator& )     = delete;
        CommandOperator& operator=( CommandOperator&& ) noexcept = delete;

        void end_recording( );

        void begin_rendering( VkRenderingInfo const& ) const;
        void end_rendering( ) const;

        void insert_barrier( VkDependencyInfo const& ) const;

        void set_viewport( VkViewport const& ) const;
        void set_viewport( std::span<VkViewport> ) const;
        void set_scissor( VkRect2D const& ) const;
        void set_scissor( std::span<VkRect2D> ) const;

        void bind_pipeline( VkPipelineBindPoint, Pipeline const& ) const;
        void bind_vertex_buffers( Buffer const&, VkDeviceSize offset ) const;
        void bind_index_buffer( Buffer const&, VkDeviceSize offset ) const;
        void bind_descriptor_set( VkPipelineBindPoint, Pipeline const&, VkDescriptorSet ) const;

        void push_constants( VkPipelineLayout, VkShaderStageFlags, uint32_t offset, uint32_t size, void const* data ) const;

        void draw_indexed( uint32_t index_count, uint32_t instance_count, uint32_t index_offset = 0, int32_t vertex_offset = 0,
                           uint32_t instance_offset = 0 ) const;

        void copy_buffer_to_image( Buffer const& src, Image const& dst, VkBufferImageCopy const& ) const;
        void copy_buffer( Buffer const& src, Buffer const& dst ) const;

    private:
        VkCommandBuffer const command_buffer_{ VK_NULL_HANDLE };
        bool recording_{ false };

    };

}


#endif //!COMMANDOPERATOR_H
