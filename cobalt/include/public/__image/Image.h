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
        VkExtent2D extent{ 0u, 0u };
        VkFormat format{ VK_FORMAT_UNDEFINED };
        VkImageTiling tiling{ VK_IMAGE_TILING_OPTIMAL };
        VkImageUsageFlags usage{ VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM };
        VkMemoryPropertyFlags properties{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
        VkImageCreateFlags create_flags{ 0 };
        VkImageAspectFlags aspect_flags{ VK_IMAGE_ASPECT_NONE };
        uint32_t layers{ 1 };
        VkImageViewType view_type{ VK_IMAGE_VIEW_TYPE_2D };
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
        [[nodiscard]] ImageView& view_at( uint32_t view_index ) const;

        [[nodiscard]] uint32_t view_count( ) const;
        [[nodiscard]] VkFormat format( ) const;
        [[nodiscard]] VkExtent2D extent( ) const;

        void transition_layout( ImageLayoutTransition const&, CommandPool& cmd_pool );
        void transition_layout( ImageLayoutTransition, CommandOperator const& cmd_operator );

    private:
        DeviceSet const& device_ref_;

        VkFormat const format_;
        VkExtent2D const extent_;
        uint32_t const layers_;

        VkImageLayout layout_{ VK_IMAGE_LAYOUT_UNDEFINED };

        VkImage image_{ VK_NULL_HANDLE };
        VkDeviceMemory image_memory_{ VK_NULL_HANDLE };

        std::vector<std::unique_ptr<ImageView>> views_{};

        void init_image( ImageCreateInfo const& create_info );
        void init_view( ImageViewCreateInfo const& create_info );

    };

}


#endif //!IMAGE_H
