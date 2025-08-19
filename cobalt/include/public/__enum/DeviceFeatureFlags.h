#ifndef DEVICEFEATUREFLAGS_H
#define DEVICEFEATUREFLAGS_H

#include <__meta/enum_traits.h>

#include <cstdint>


namespace cobalt
{
    enum class DeviceFeatureFlags : uint32_t
    {
        NONE                                    = 0,
        FAMILIES_INDICES_SUITABLE               = 1 << 0,
        ANISOTROPIC_SAMPLING                    = 1 << 1,
        SWAPCHAIN_EXT                           = 1 << 2,
        DYNAMIC_RENDERING_EXT                   = 1 << 3,
        SYNCHRONIZATION_2_EXT                   = 1 << 4,
        SHADER_IMAGE_ARRAY_NON_UNIFORM_INDEXING = 1 << 5,
    };

    template <>
    struct meta::enable_enum_flags<DeviceFeatureFlags> : std::true_type { };

}


#endif //!DEVICEFEATUREFLAGS_H
