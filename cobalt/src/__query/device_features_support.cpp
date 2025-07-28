#include <__query/device_features_support.h>


namespace cobalt::query
{
    bool supports_anisotropy( VkPhysicalDeviceFeatures const& features )
    {
        return features.samplerAnisotropy;
    }

}
