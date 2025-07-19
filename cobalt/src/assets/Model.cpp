#include <CobaltVK.h>
#include <assets/Model.h>

#include <builders/ModelLoader.h>

#include <assets/Buffer.h>


namespace cobalt
{
    Model::~Model( )
    {
        auto const& instance = CVK.get_vk_instance( );
        if ( vertex_buffer_ != VK_NULL_HANDLE )
        {
            vkDestroyBuffer( instance.device( ).logical( ), vertex_buffer_, nullptr );
        }
        if ( vertex_buffer_memory_ != VK_NULL_HANDLE )
        {
            vkFreeMemory( instance.device( ).logical( ), vertex_buffer_memory_, nullptr );
        }
    }


    void Model::create_vertex_buffer( VkDevice const device, VkPhysicalDevice const physical_device,
                                      VkCommandPool const command_pool, VkQueue const graphics_queue )
    {
        VkDeviceSize const buffer_size = sizeof( vertices_[0] ) * vertices_.size( );
        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;

        // We're now using a new stagingBuffer with stagingBufferMemory for mapping and copying the vertex data. This will
        // load vertex data from high performance memory.
        // - VK_BUFFER_USAGE_TRANSFER_SRC_BIT: Buffer can be used as source in a memory transfer operation.
        // - VK_BUFFER_USAGE_TRANSFER_DST_BIT: Buffer can be used as destination in a memory transfer operation.
        Buffer::vk_create_buffer( device, physical_device, buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  staging_buffer, staging_buffer_memory );

        // It is now time to copy the vertex data to the buffer. This is done by mapping the buffer memory into CPU
        // accessible memory. memcpy the vertex data to the mapped memory and unmap it again.
        void* data;
        vkMapMemory( device, staging_buffer_memory, 0, buffer_size, 0, &data );

        // ReSharper disable once CppRedundantCastExpression
        memcpy( data, vertices_.data( ), static_cast<size_t>( buffer_size ) );

        vkUnmapMemory( device, staging_buffer_memory );

        Buffer::vk_create_buffer( device, physical_device, buffer_size,
                                  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer_, vertex_buffer_memory_ );

        Buffer::vk_copy_buffer( device, command_pool, graphics_queue, staging_buffer, vertex_buffer_, buffer_size );

        // We can now destroy the staging buffer and free its memory.
        vkDestroyBuffer( device, staging_buffer, nullptr );
        vkFreeMemory( device, staging_buffer_memory, nullptr );
    }

}
