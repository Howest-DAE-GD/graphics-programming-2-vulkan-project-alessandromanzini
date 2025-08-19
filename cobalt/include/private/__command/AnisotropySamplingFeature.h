#ifndef ANISOTROPYSAMPLINGFEATURE_H
#define ANISOTROPYSAMPLINGFEATURE_H

#include "FeatureCommand.h"


namespace cobalt::exe
{
    class AnisotropySamplingFeature final : public FeatureCommand
    {
    public:
        bool validate( ValidationData const& data ) const override
        {
            return data.features.features.samplerAnisotropy;
        }


        void enable( EnableData& data ) override
        {
            data.features.features.samplerAnisotropy = VK_TRUE;
        }

    };


}


#endif //!ANISOTROPYSAMPLINGFEATURE_H
