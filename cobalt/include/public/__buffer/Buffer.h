#ifndef BUFFER_H
#define BUFFER_H

#include <__memory/Resource.h>

#include <__enum/BufferContentType.h>

#include <vulkan/vulkan_core.h>

#include <span>


namespace cobalt
{
    class DeviceSet;
    class CommandPool;
    class Image;
}

namespace cobalt
{
    class Buffer final : public memory::Resource
    {
    public:
        explicit Buffer( DeviceSet const&, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                         buffer::BufferContentType content_type = buffer::BufferContentType::ANY );
        ~Buffer( ) noexcept override;

        Buffer( Buffer&& ) noexcept;
        Buffer( const Buffer& )                = delete;
        Buffer& operator=( const Buffer& )     = delete;
        Buffer& operator=( Buffer&& ) noexcept = delete;

        [[nodiscard]] VkBuffer handle( ) const;
        [[nodiscard]] VkDeviceMemory memory( ) const;

        [[nodiscard]] buffer::BufferContentType content_type( ) const;
        [[nodiscard]] VkDeviceSize buffer_size( ) const;
        [[nodiscard]] VkDeviceSize memory_size( ) const;

        [[nodiscard]] void* data( ) const;

        void map_memory( VkDeviceSize offset = 0, VkMemoryMapFlags flags = 0 );
        void unmap_memory( );

        void copy_to( Buffer const& dst, CommandPool& cmd_pool ) const;
        void copy_to( Image const& dst, CommandPool& cmd_pool ) const;

    private:
        DeviceSet const& device_ref_;

        buffer::BufferContentType const content_type_;

        VkDeviceSize const buffer_size_{};
        VkDeviceSize memory_size_{};

        VkBuffer buffer_{ VK_NULL_HANDLE };
        VkDeviceMemory buffer_memory_{ VK_NULL_HANDLE };
        void* memory_map_ptr_{ nullptr };

        // todo: might want to upgrade to VkMemoryRequirements2
        [[nodiscard]] VkMemoryRequirements fetch_memory_requirements( ) const;

    };

    namespace buffer
    {
        namespace internal
        {
            [[nodiscard]] Buffer allocate_data_buffer( DeviceSet const&, CommandPool&, void const* data, VkDeviceSize size,
                                                       VkBufferUsageFlags main_usage_bit, BufferContentType content_type );
        }

        [[nodiscard]] Buffer make_staging_buffer( DeviceSet const&, VkDeviceSize size );
        [[nodiscard]] Buffer make_uniform_buffer( DeviceSet const&, VkDeviceSize size );


        template <typename v_t>
        [[nodiscard]] Buffer make_vertex_buffer( DeviceSet const& device, CommandPool& cmd_pool, std::span<v_t const> vertices )
        {
            return internal::allocate_data_buffer(
                device, cmd_pool, vertices.data( ), vertices.size_bytes( ),
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, BufferContentType::VERTEX );
        }


        template <typename i_t>
        [[nodiscard]] Buffer make_index_buffer( DeviceSet const& device, CommandPool& cmd_pool, std::span<i_t const> indices )
        {
            return internal::allocate_data_buffer(
                device, cmd_pool, indices.data( ), indices.size_bytes( ),
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT, to_buffer_content_type<i_t>( ) );
        }

    }

}


#endif //!BUFFER_H
