#ifndef DYNAMICRENDERINGFEATURE_H
#define DYNAMICRENDERINGFEATURE_H

#include "FeatureCommand.h"


namespace cobalt::exe
{
    class DynamicRenderingFeature final : public FeatureCommand
    {
    public:
        bool validate( ValidationData const& data ) const override
        {
            return has_extension( data.extensions, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME );
        }


        void enable( EnableData& data ) override
        {
            data.extensions.emplace_back( VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME );
            data.features13.dynamicRendering = VK_TRUE;
        }

    };

}


#endif //!DYNAMICRENDERINGFEATURE_H
