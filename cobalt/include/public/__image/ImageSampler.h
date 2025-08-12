#ifndef IMAGESAMPLER_H
#define IMAGESAMPLER_H

#include <__memory/Resource.h>

#include <vulkan/vulkan_core.h>


namespace cobalt
{
    class DeviceSet;
}

namespace cobalt
{
    struct ImageSamplerCreateInfo
    {
        VkFilter filter{ VK_FILTER_MAX_ENUM };
        VkSamplerAddressMode address_mode{ VK_SAMPLER_ADDRESS_MODE_MAX_ENUM };
        VkSamplerMipmapMode mipmap_mode{ VK_SAMPLER_MIPMAP_MODE_MAX_ENUM };
        VkBorderColor border_color{ VK_BORDER_COLOR_MAX_ENUM };
        bool unnormalized_coordinates{ false };
        bool compare_enable{ false };
        VkCompareOp compare_op{ VK_COMPARE_OP_MAX_ENUM };

    };


    class ImageSampler final : public memory::Resource
    {
    public:
        ImageSampler( DeviceSet const&, ImageSamplerCreateInfo const& );
        ~ImageSampler( ) noexcept override;

        ImageSampler( const ImageSampler& )                = delete;
        ImageSampler( ImageSampler&& ) noexcept            = delete;
        ImageSampler& operator=( const ImageSampler& )     = delete;
        ImageSampler& operator=( ImageSampler&& ) noexcept = delete;

        [[nodiscard]] VkSampler handle( ) const noexcept;

    private:
        DeviceSet const& device_ref_;
        VkSampler texture_sampler_{ VK_NULL_HANDLE };

    };

}


#endif //!IMAGESAMPLER_H
