#include <__image/ImageSampler.h>

#include <__context/DeviceSet.h>
#include <__validation/result.h>


namespace cobalt
{
    ImageSampler::ImageSampler( DeviceSet const& device, ImageSamplerCreateInfo const& create_info )
        : device_ref_{ device }
    {
        VkPhysicalDeviceProperties2 properties{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2
        };
        vkGetPhysicalDeviceProperties2( device_ref_.physical( ), &properties );

        VkSamplerCreateInfo const sampler_create_info{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,

            .magFilter = create_info.filter,
            .minFilter = create_info.filter,

            .addressModeU = create_info.address_mode,
            .addressModeV = create_info.address_mode,
            .addressModeW = create_info.address_mode,

            .anisotropyEnable = device_ref_.has_feature( DeviceFeatureFlags::ANISOTROPIC_SAMPLING ),
            .maxAnisotropy = properties.properties.limits.maxSamplerAnisotropy,

            .borderColor = create_info.border_color,

            .unnormalizedCoordinates = create_info.unnormalized_coordinates,

            .compareEnable = create_info.compare_enable,
            .compareOp = create_info.compare_op,

            .mipmapMode = create_info.mipmap_mode,
            .mipLodBias = 0.f,
            .minLod = 0.f,
            .maxLod = 0.f
        };

        validation::throw_on_bad_result(
            vkCreateSampler( device_ref_.logical( ), &sampler_create_info, nullptr, &texture_sampler_ ),
            "failed to create texture sampler!" );
    }


    ImageSampler::~ImageSampler( ) noexcept
    {
        vkDestroySampler( device_ref_.logical( ), texture_sampler_, nullptr );
    }


    VkSampler ImageSampler::handle( ) const noexcept
    {
        return texture_sampler_;
    }

}
