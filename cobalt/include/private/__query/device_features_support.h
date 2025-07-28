#ifndef DEVICE_FEATURES_SUPPORT_H
#define DEVICE_FEATURES_SUPPORT_H

#include <vulkan/vulkan_core.h>


namespace cobalt::query
{
    [[nodiscard]] bool supports_anisotropy( VkPhysicalDeviceFeatures const& features );

}


#endif //!DEVICE_FEATURES_SUPPORT_H
