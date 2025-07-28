#ifndef DEVICEFEATUREFLAGS_H
#define DEVICEFEATUREFLAGS_H

#include <cstdint>


namespace cobalt
{
    enum class DeviceFeatureFlags : uint32_t
    {
        NONE                        = 0,
        FAMILIES_INDICES_SUITABLE   = 1 << 0,
        ANISOTROPIC_SAMPLING        = 1 << 1,
        SWAPCHAIN_EXTENSION         = 1 << 2,
        DYNAMIC_RENDERING_EXTENSION = 1 << 3,
    };

    // +---------------------------+
    // | BITWISE OPS               |
    // +---------------------------+
    [[nodiscard]] DeviceFeatureFlags operator|( DeviceFeatureFlags lhs, DeviceFeatureFlags rhs );
    [[nodiscard]] DeviceFeatureFlags operator&( DeviceFeatureFlags lhs, DeviceFeatureFlags rhs );
    [[nodiscard]] DeviceFeatureFlags operator~( DeviceFeatureFlags flag );

    [[nodiscard]] bool any( DeviceFeatureFlags flag );

}


#endif //!DEVICEFEATUREFLAGS_H
