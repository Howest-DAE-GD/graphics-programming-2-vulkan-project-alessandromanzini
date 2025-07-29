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
        SWAPCHAIN_EXT               = 1 << 2,
        SWAPCHAIN_MAINTENANCE_1_EXT = 1 << 3,
        DYNAMIC_RENDERING_EXT       = 1 << 4,
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
