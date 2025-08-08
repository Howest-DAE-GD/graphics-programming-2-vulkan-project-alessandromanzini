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


    struct TextureSamplerCreateInfo
    {
        VkFilter filter{ VK_FILTER_LINEAR };
        VkSamplerAddressMode address_mode{ VK_SAMPLER_ADDRESS_MODE_REPEAT };
        VkSamplerMipmapMode mipmap_mode{ VK_SAMPLER_MIPMAP_MODE_LINEAR };
        VkBorderColor border_color{ VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK };
        bool unnormalized_coordinates{ false };
    };


    class TextureImage final : public memory::Resource
    {
    public:
        explicit TextureImage( DeviceSet const&, CommandPool&, TextureImageCreateInfo const&, TextureSamplerCreateInfo const& );
        ~TextureImage( ) noexcept override;

        TextureImage( const TextureImage& )                = delete;
        TextureImage( TextureImage&& ) noexcept            = delete;
        TextureImage& operator=( const TextureImage& )     = delete;
        TextureImage& operator=( TextureImage&& ) noexcept = delete;

        [[nodiscard]] Image const& image( ) const;
        [[nodiscard]] VkSampler sampler( ) const;

    private:
        DeviceSet const& device_ref_;

        std::unique_ptr<Image> texture_image_ptr_{ nullptr };
        VkSampler texture_sampler_{ VK_NULL_HANDLE };

        void create_image( TextureImageCreateInfo const&, CommandPool& );
        void create_sampler( TextureSamplerCreateInfo const& );

    };

}


#endif //!TEXTUREIMAGE_H
