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
        VkPhysicalDeviceFeatures2 features;
        std::vector<VkExtensionProperties> extensions;
        std::vector<char const*> extensions_selection;
    };

    using validation_fn_t = bool( * )( ValidationCallbackData& );
    using validation_arg_t = ValidationCallbackData;

    [[nodiscard]] bool is_family_indices_suitable( ValidationCallbackData& data );
    [[nodiscard]] bool is_swapchain_adequate( ValidationCallbackData& data );
    [[nodiscard]] bool is_swapchain_maintenance_1_supported( ValidationCallbackData& data );
    [[nodiscard]] bool is_anisotropy_sampler_supported( ValidationCallbackData& data );
    [[nodiscard]] bool is_independent_blend_supported( ValidationCallbackData& data );
    [[nodiscard]] bool is_dynamic_rendering_supported( ValidationCallbackData& data );
    [[nodiscard]] bool is_synchronization_2_supported( ValidationCallbackData& data );
    [[nodiscard]] bool is_vertex_input_dynamic_state_supported( ValidationCallbackData& data );
    [[nodiscard]] bool is_cubic_filter_supported( ValidationCallbackData& data );
    [[nodiscard]] bool is_image_array_non_uniform_indexing_supported( ValidationCallbackData& data );

}


#endif //!DEVICE_FEATURES_H
