#ifndef TEXTUREIMAGE_H
#define TEXTUREIMAGE_H

#include <__memory/Resource.h>

#include <vulkan/vulkan_core.h>

#include <filesystem>


namespace cobalt
{
    class DeviceSet;
    class CommandPool;
    class Image;
}

namespace cobalt
{
    struct TextureImageCreateInfo
    {
        std::filesystem::path const& path_to_img{};
        VkFormat image_format{ VK_FORMAT_UNDEFINED };
    };

    class TextureImage final : public memory::Resource
    {
    public:
        explicit TextureImage( DeviceSet const&, CommandPool&, TextureImageCreateInfo const& );
        ~TextureImage( ) noexcept override;

        TextureImage( TextureImage&& ) noexcept;
        TextureImage( const TextureImage& )                = delete;
        TextureImage& operator=( const TextureImage& )     = delete;
        TextureImage& operator=( TextureImage&& ) noexcept = delete;

        [[nodiscard]] Image const& image( ) const;
        [[nodiscard]] VkSampler sampler( ) const;

    private:
        DeviceSet const& device_ref_;
        std::unique_ptr<Image> texture_image_ptr_{ nullptr };

    };

}


#endif //!TEXTUREIMAGE_H
