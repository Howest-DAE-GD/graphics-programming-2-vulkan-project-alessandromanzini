#ifndef FEATURECOMMAND_H
#define FEATURECOMMAND_H

#include <__context/InstanceBundle.h>
#include <__validation/compare.h>


namespace cobalt::exe
{
    struct ValidationData
    {
        InstanceBundle const* instance;
        VkPhysicalDevice device;
        VkPhysicalDeviceFeatures2 features;
        std::vector<VkExtensionProperties> extensions;
    };


    struct EnableData
    {
        VkPhysicalDeviceFeatures2 features;
        VkPhysicalDeviceVulkan11Features features11;
        VkPhysicalDeviceVulkan12Features features12;
        VkPhysicalDeviceVulkan13Features features13;
        std::vector<char const*> extensions;


        void map_data( )
        {
            features.pNext   = &features11;
            features11.pNext = &features12;
            features12.pNext = &features13;
            features13.pNext = nullptr;
        }
    };


    class FeatureCommand
    {
    public:
        FeatureCommand( ) = default;
        virtual ~FeatureCommand( ) noexcept = default;

        FeatureCommand( const FeatureCommand& )                = delete;
        FeatureCommand( FeatureCommand&& ) noexcept            = delete;
        FeatureCommand& operator=( const FeatureCommand& )     = delete;
        FeatureCommand& operator=( FeatureCommand&& ) noexcept = delete;

        virtual bool validate( ValidationData const& ) const = 0;
        virtual void enable( EnableData& ) = 0;

    protected:
        [[nodiscard]] static bool has_extension( std::vector<VkExtensionProperties> const& extensions, char const* ext_name )
        {
            return validation::contains_required( ext_name, extensions, std::mem_fn( &VkExtensionProperties::extensionName ) );
        }

    };

}


#endif //!FEATURECOMMAND_H
