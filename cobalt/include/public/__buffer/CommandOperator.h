#ifndef COMMANDOPERATOR_H
#define COMMANDOPERATOR_H

#include <vulkan/vulkan_core.h>

#include <span>


namespace cobalt
{
    class Pipeline;
    class Image;
    class Buffer;
    class PipelineLayout;
}

namespace cobalt
{
    // todo: make all size requesting methods templated to get the size
    class CommandOperator final
    {
    public:
        CommandOperator( VkCommandBuffer command_buffer, VkCommandBufferBeginInfo const& );
        ~CommandOperator( ) noexcept;

        CommandOperator( CommandOperator&& ) noexcept;
        CommandOperator( const CommandOperator& )                = delete;
        CommandOperator& operator=( const CommandOperator& )     = delete;
        CommandOperator& operator=( CommandOperator&& ) noexcept = delete;

        void store_render_area( VkRect2D const& );
        void store_viewport( VkViewport const& );

        void end_recording( );

        void begin_rendering(
            std::span<VkRenderingAttachmentInfo const> color_attachments, VkRenderingAttachmentInfo const* depth_attachment,
            std::optional<VkRect2D> const& render_area_override = std::nullopt ) const;
        void end_rendering( ) const;

        void insert_barrier( VkDependencyInfo const& ) const;

        void set_viewport( std::optional<VkViewport> const& viewport_override = std::nullopt ) const;
        void set_scissor( std::optional<VkRect2D> const& scissor_override = std::nullopt ) const;

        void bind_pipeline( Pipeline const&, uint32_t frame_index ) const;

        void bind_vertex_buffers( Buffer const&, VkDeviceSize offset ) const;
        void bind_index_buffer( Buffer const&, VkDeviceSize offset ) const;

        void push_constants( Pipeline const&, VkShaderStageFlags, uint32_t offset, uint32_t size, void const* data ) const;

        void draw( int32_t vertex_count, uint32_t instance_count,
                   uint32_t vertex_offset = 0u, uint32_t instance_offset = 0u ) const;
        void draw_indexed( uint32_t index_count, uint32_t instance_count, uint32_t index_offset = 0u,
                           int32_t vertex_offset = 0u, uint32_t instance_offset = 0u ) const;

        void copy_buffer_to_image( Buffer const& src, Image const& dst, VkBufferImageCopy const& ) const;
        void copy_buffer( Buffer const& src, Buffer const& dst ) const;

    private:
        VkCommandBuffer const command_buffer_{ VK_NULL_HANDLE };
        bool recording_{ false };

        VkRect2D render_area_{ .offset = { 0u, 0u }, .extent = { 0u, 0u } };
        VkViewport viewport_{ .x = 0.f, .y = 0.f, .width = 0.f, .height = 0.f, .minDepth = 0.f, .maxDepth = 0.f };

    };

}


#endif //!COMMANDOPERATOR_H
