#ifndef DEVICE_FEATURES_H
#define DEVICE_FEATURES_H

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt
{
    class InstanceBundle;
}

namespace cobalt::validation
{
    struct ValidationCallbackData
    {
        InstanceBundle const& instance;
        VkPhysicalDevice device;
        VkPhysicalDeviceFeatures features;
        std::vector<VkExtensionProperties> extensions;
        std::vector<char const*> extensions_selection;
    };

    using validation_fn_t = bool( * )( ValidationCallbackData& );
    using validation_arg_t = ValidationCallbackData;

    [[nodiscard]] bool is_family_indices_suitable( ValidationCallbackData& data );
    [[nodiscard]] bool is_swapchain_adequate( ValidationCallbackData& data );
    [[nodiscard]] bool is_anisotropy_sampler_supported( ValidationCallbackData& data );
    [[nodiscard]] bool is_dynamic_rendering_supported( ValidationCallbackData& data );

}


#endif //!DEVICE_FEATURES_H
