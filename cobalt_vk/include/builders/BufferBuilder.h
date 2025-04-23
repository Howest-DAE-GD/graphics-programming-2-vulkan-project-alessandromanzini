#ifndef BUFFERBUILDER_H
#define BUFFERBUILDER_H

#include <assets/Buffer.h>

#include <vulkan/vulkan_core.h>


namespace cobalt_vk::builders
{
    namespace transfer_ops
    {
        struct BufferToBuffer
        {
            BufferToBuffer( VkCommandPool commandPool, VkQueue graphicsQueue );

            void operator()( VkDevice device, const Buffer& src, const Buffer& dst ) const;

            const VkCommandPool commandPool;
            const VkQueue graphicsQueue;
        };

        struct BufferToImage
        {
            BufferToImage( VkCommandPool commandPool, VkQueue graphicsQueue );

            void operator()( VkDevice device, const Buffer& src, const Buffer& dst ) const;

            const VkCommandPool commandPool;
            const VkQueue graphicsQueue;
        };

    }

    class BufferBuilder
    {
    public:
        BufferBuilder( const BufferBuilder& )                = delete;
        BufferBuilder( BufferBuilder&& ) noexcept            = delete;
        BufferBuilder& operator=( const BufferBuilder& )     = delete;
        BufferBuilder& operator=( BufferBuilder&& ) noexcept = delete;

    protected:
        const VkDeviceSize buffer_size_{};

        explicit BufferBuilder( VkDeviceSize bufferSize );

    };


    class SrcBufferBuilder final : public BufferBuilder
    {
    public:
        explicit SrcBufferBuilder( VkDeviceSize bufferSize );
        void build( VkDevice device, VkPhysicalDevice physicalDevice, Buffer& buffer ) const;

    };


    class DstBufferBuilder final : public BufferBuilder
    {
    public:
        explicit DstBufferBuilder( VkDeviceSize bufferSize );
        void build( VkDevice device, VkPhysicalDevice physicalDevice, Buffer& buffer ) const;

    };


    class DataBufferBuilder final : public BufferBuilder
    {
    public:
        explicit DataBufferBuilder( void* data, VkDeviceSize bufferSize );
        void build( VkDevice device, VkPhysicalDevice physicalDevice, transfer_ops::BufferToBuffer op, Buffer& buffer ) const;
        // void build( VkDevice device, VkPhysicalDevice physicalDevice, transfer_ops::BufferToImage op, Buffer& buffer ) const;

    private:
        void* data_{ nullptr };

        [[nodiscard]] Buffer build_staging_buffer( VkDevice device, VkPhysicalDevice physicalDevice ) const;

    };

}


#endif //!BUFFERBUILDER_H
