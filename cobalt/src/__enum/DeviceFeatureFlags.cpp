#include <__enum/DeviceFeatureFlags.h>


namespace cobalt
{
    DeviceFeatureFlags operator|( DeviceFeatureFlags lhs, DeviceFeatureFlags rhs )
    {
        return static_cast<DeviceFeatureFlags>( static_cast<uint32_t>( lhs ) | static_cast<uint32_t>( rhs ) );
    }


    DeviceFeatureFlags operator&( DeviceFeatureFlags lhs, DeviceFeatureFlags rhs )
    {
        return static_cast<DeviceFeatureFlags>( static_cast<uint32_t>( lhs ) & static_cast<uint32_t>( rhs ) );
    }


    DeviceFeatureFlags operator~( DeviceFeatureFlags flag )
    {
        return static_cast<DeviceFeatureFlags>( ~static_cast<uint32_t>( flag ) );
    }


    bool any( DeviceFeatureFlags const flag )
    {
        return flag != DeviceFeatureFlags::NONE;
    }

}
