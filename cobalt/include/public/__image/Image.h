#ifndef IMAGE_H
#define IMAGE_H

#include <__memory/Resource.h>

#include <__image/ImageLayoutTransition.h>
#include <__image/ImageView.h>

#include <vulkan/vulkan_core.h>

#include <memory>


namespace cobalt
{
    class DeviceSet;
    class CommandPool;
    class CommandOperator;
}

namespace cobalt
{
    struct ImageCreateInfo
    {
        VkExtent2D extent{ 0, 0 };
        VkFormat format{ VK_FORMAT_UNDEFINED };
        VkImageTiling tiling{ VK_IMAGE_TILING_MAX_ENUM };
        VkImageUsageFlags usage{ VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM };
        VkMemoryPropertyFlags properties{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
        VkImageAspectFlags aspect_flags{ VK_IMAGE_ASPECT_NONE };
    };


    class Image final : public memory::Resource
    {
    public:
        explicit Image( DeviceSet const&, ImageCreateInfo const& );
        explicit Image( DeviceSet const&, VkExtent2D extent, ImageViewCreateInfo const& );
        ~Image( ) override;

        Image( Image&& ) noexcept;
        Image( const Image& )                = delete;
        Image& operator=( const Image& )     = delete;
        Image& operator=( Image&& ) noexcept = delete;

        [[nodiscard]] VkImage handle( ) const;
        [[nodiscard]] ImageView& view( ) const;

        [[nodiscard]] VkFormat format( ) const;
        [[nodiscard]] VkExtent2D extent( ) const;

        void transition_layout( ImageLayoutTransition const&, CommandPool& cmd_pool );
        void transition_layout( ImageLayoutTransition, CommandOperator const& cmd_operator );

    private:
        DeviceSet const& device_ref_;

        VkFormat const format_{ VK_FORMAT_UNDEFINED };
        VkExtent2D const extent_{ 0, 0 };

        VkImageLayout layout_{ VK_IMAGE_LAYOUT_UNDEFINED };

        VkImage image_{ VK_NULL_HANDLE };
        VkDeviceMemory image_memory_{ VK_NULL_HANDLE };
        std::unique_ptr<ImageView> view_ptr_{ nullptr };

        void init_image( ImageCreateInfo const& create_info );
        void init_view( ImageViewCreateInfo const& create_info );

    };

}


#endif //!IMAGE_H
