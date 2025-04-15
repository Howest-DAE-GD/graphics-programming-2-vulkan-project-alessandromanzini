#include <Buffer.h>
#include <assets/Model.h>

#include <builders/ModelLoader.h>

#include <assets/Buffer.h>


namespace cobalt_vk
{
    void Model::create_vertex_buffer( const VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue )
    {
        const VkDeviceSize bufferSize = sizeof( vertices_[0] ) * vertices_.size( );
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        // We're now using a new stagingBuffer with stagingBufferMemory for mapping and copying the vertex data. This will
        // load vertex data from high performance memory.
        // - VK_BUFFER_USAGE_TRANSFER_SRC_BIT: Buffer can be used as source in a memory transfer operation.
        // - VK_BUFFER_USAGE_TRANSFER_DST_BIT: Buffer can be used as destination in a memory transfer operation.
        Buffer::vk_create_buffer( device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          stagingBuffer, stagingBufferMemory );

        // It is now time to copy the vertex data to the buffer. This is done by mapping the buffer memory into CPU
        // accessible memory. memcpy the vertex data to the mapped memory and unmap it again.
        void* data;
        vkMapMemory( device, stagingBufferMemory, 0, bufferSize, 0, &data );

        // ReSharper disable once CppRedundantCastExpression
        memcpy( data, vertices_.data( ), static_cast<size_t>( bufferSize ) );

        vkUnmapMemory( device, stagingBufferMemory );

        Buffer::vk_create_buffer( device, physicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer_, vertex_buffer_memory_ );

        Buffer::vk_copy_buffer(device, commandPool, graphicsQueue, stagingBuffer, vertex_buffer_, bufferSize );

        // We can now destroy the staging buffer and free its memory.
        vkDestroyBuffer( device, stagingBuffer, nullptr );
        vkFreeMemory( device, stagingBufferMemory, nullptr );
    }


    void Model::release( const VkDevice device )
    {
        if ( vertex_buffer_ != VK_NULL_HANDLE )
        {
            vkDestroyBuffer( device, vertex_buffer_, nullptr );
        }
        if ( vertex_buffer_memory_ != VK_NULL_HANDLE )
        {
            vkFreeMemory( device, vertex_buffer_memory_, nullptr );
        }
    }

}
