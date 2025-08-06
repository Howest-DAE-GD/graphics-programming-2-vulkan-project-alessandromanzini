#ifndef BUFFER_H
#define BUFFER_H

#include <__memory/Resource.h>

#include <vulkan/vulkan_core.h>


namespace cobalt
{
    class DeviceSet;
}

namespace cobalt
{
    class Buffer final : public memory::Resource
    {
    public:
        explicit Buffer( DeviceSet const&, VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags );
        ~Buffer( ) noexcept override;

        Buffer( Buffer&& ) noexcept;
        Buffer( const Buffer& )                = delete;
        Buffer& operator=( const Buffer& )     = delete;
        Buffer& operator=( Buffer&& ) noexcept = delete;

        [[nodiscard]] VkBuffer handle( ) const;
        [[nodiscard]] VkDeviceMemory memory( ) const;

        [[nodiscard]] VkDeviceSize buffer_size( ) const;
        [[nodiscard]] VkDeviceSize memory_size( ) const;

        [[nodiscard]] void* data( ) const;

        void map_memory( VkDeviceSize offset = 0, VkMemoryMapFlags = 0 );
        void unmap_memory( );

    private:
        DeviceSet const& device_ref_;

        VkDeviceSize const buffer_size_{};
        VkDeviceSize memory_size_{};

        VkBuffer buffer_{ VK_NULL_HANDLE };
        VkDeviceMemory buffer_memory_{ VK_NULL_HANDLE };
        void* memory_map_ptr_{ nullptr };

        // todo: might want to upgrade to VkMemoryRequirements2
        [[nodiscard]] VkMemoryRequirements fetch_memory_requirements( ) const;

    };

}


#endif //!BUFFER_H
