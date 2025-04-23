#include <BufferBuilder.h>
#include <DeletionQueue.h>
#include <../../include/ResourcePool.h>

#include <queries/memory_queries.h>
#include <validation/result.h>


namespace cobalt_vk::builders
{
    // +---------------------------+
    // | BUFFER BUILDER            |
    // +---------------------------+
    BufferBuilder::BufferBuilder( const VkDeviceSize bufferSize ) : buffer_size_{ bufferSize } { }


    // +---------------------------+
    // | SRC BUFFER BUILDER        |
    // +---------------------------+
    SrcBufferBuilder::SrcBufferBuilder( const VkDeviceSize bufferSize ) : BufferBuilder( bufferSize ) { }


    void SrcBufferBuilder::build( const VkDevice device, const VkPhysicalDevice physicalDevice, Buffer& buffer ) const
    {
        buffer.initialize( device, physicalDevice, buffer_size_,
                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
    }


    // +---------------------------+
    // | DST BUFFER BUILDER        |
    // +---------------------------+
    DstBufferBuilder::DstBufferBuilder( const VkDeviceSize bufferSize ) : BufferBuilder( bufferSize ) { }


    void DstBufferBuilder::build( const VkDevice device, const VkPhysicalDevice physicalDevice, Buffer& buffer ) const
    {
        buffer.initialize( device, physicalDevice, buffer_size_,
                           VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
    }


    // +---------------------------+
    // | INDEX BUFFER BUILDER      |
    // +---------------------------+
    DataBufferBuilder::DataBufferBuilder( void* data, const VkDeviceSize bufferSize )
        : BufferBuilder( bufferSize )
        , data_{ data } { }


    void DataBufferBuilder::build( const VkDevice device, const VkPhysicalDevice physicalDevice, const transfer_ops::BufferToBuffer op,
        Buffer& buffer ) const
    {
        // Create staging buffer
        Buffer stagingBuffer = build_staging_buffer( device, physicalDevice );

        // Create the index buffer and transfer the data from the staging buffer to the index buffer
        DstBufferBuilder{ buffer_size_ }.build( device, physicalDevice, buffer );
        ResourcePool::get_instance( ).register_resource( buffer, device );

        // Copy the data from the staging buffer to the index buffer and release the staging buffer
        op( device, stagingBuffer, buffer );
        stagingBuffer.release( device );
    }


    Buffer DataBufferBuilder::build_staging_buffer( const VkDevice device, const VkPhysicalDevice physicalDevice ) const
    {
        Buffer stagingBuffer{};
        SrcBufferBuilder{ buffer_size_ }.build( device, physicalDevice, stagingBuffer );

        // Map the buffer and copy the index data into it
        void* data;
        stagingBuffer.map_memory( device, 0, buffer_size_, 0, &data );
        memcpy( data, data_, buffer_size_ );
        stagingBuffer.unmap_memory( device );

        return stagingBuffer;
    }


    namespace transfer_ops
    {
        BufferToBuffer::BufferToBuffer( const VkCommandPool commandPool, const VkQueue graphicsQueue )
            : commandPool{ commandPool }
            , graphicsQueue{ graphicsQueue } { }


        void BufferToBuffer::operator()( const VkDevice device, const Buffer& src, const Buffer& dst ) const
        {
            Buffer::copy_buffer( device, commandPool, graphicsQueue, src, dst );
        }

    }

}
