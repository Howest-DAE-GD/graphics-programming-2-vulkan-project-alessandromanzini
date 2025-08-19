#ifndef SYNCHRONIZATION2_FEATURE_H
#define SYNCHRONIZATION2_FEATURE_H

#include "FeatureCommand.h"


namespace cobalt::exe
{
    class Synchronization2Feature final : public FeatureCommand
    {
    public:
        bool validate( ValidationData const& data ) const override
        {
            return has_extension( data.extensions, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME );
        }


        void enable( EnableData& data ) override
        {
            data.extensions.emplace_back( VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME );
            data.features13.synchronization2 = VK_TRUE;
        }

    };

}


#endif //!SYNCHRONIZATION2_FEATURE_H
