#ifndef BUFFER_H
#define BUFFER_H

#include <vulkan/vulkan_core.h>

#include <cleanup/Releasable.h>


namespace cobalt_vk
{
    class Buffer final : public cleanup::Releasable
    {
    public:
        Buffer( VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties );
        ~Buffer( ) noexcept override = default;

        Buffer( const Buffer& )                = delete;
        Buffer( Buffer&& ) noexcept;
        Buffer& operator=( const Buffer& )     = delete;
        Buffer& operator=( Buffer&& ) noexcept;

        void map_memory( VkDevice device, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** data ) const;
        void unmap_memory( VkDevice device ) const;

        // TODO> REMOVE
        [[nodiscard]] VkBuffer get_buffer( ) const { return buffer_; }

        void release( VkDevice device ) override;

        static void copy_buffer( VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, const Buffer* src, const Buffer* dst );

    private:
        VkBuffer buffer_{ VK_NULL_HANDLE };
        VkDeviceMemory buffer_memory_{ VK_NULL_HANDLE };

        VkDeviceSize size_{};

    };

}


#endif //!BUFFER_H
