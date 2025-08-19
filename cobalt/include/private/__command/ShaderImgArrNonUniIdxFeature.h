#ifndef SHADERIMAGEARRAYNONUNIFORMINDEXINGFEATURE_H
#define SHADERIMAGEARRAYNONUNIFORMINDEXINGFEATURE_H

#include "FeatureCommand.h"


namespace cobalt::exe
{
    class ShaderImgArrNonUniIdxFeature final : public FeatureCommand
    {
    public:
        bool validate( ValidationData const& data ) const override
        {
            return data.features.features.shaderSampledImageArrayDynamicIndexing;
        }


        void enable( EnableData& data ) override
        {
            data.features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        }

    };

}


#endif //!SHADERIMAGEARRAYNONUNIFORMINDEXINGFEATURE_H
