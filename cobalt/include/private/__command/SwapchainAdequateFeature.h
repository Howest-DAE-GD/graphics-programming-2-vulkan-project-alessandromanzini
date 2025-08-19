#ifndef SWAPCHAINADEQUATEFEATURE_H
#define SWAPCHAINADEQUATEFEATURE_H

#include "FeatureCommand.h"

#include "../__query/swapchain_support.h"


namespace cobalt::exe
{
    class SwapchainAdequateFeature final : public FeatureCommand
    {
    public:
        bool validate( ValidationData const& data ) const override
        {
            return has_extension( data.extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME )
                   && query::check_swapchain_support( data.device, *data.instance ).is_adequate( );
        }


        void enable( EnableData& data ) override
        {
            data.extensions.emplace_back( VK_KHR_SWAPCHAIN_EXTENSION_NAME );
        }

    };

}


#endif //!SWAPCHAINADEQUATEFEATURE_H
