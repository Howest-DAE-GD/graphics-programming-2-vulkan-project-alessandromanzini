#ifndef PHYSICALDEVICESELECTOR_H
#define PHYSICALDEVICESELECTOR_H

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt
{
    class InstanceBundle;
}

namespace cobalt::validation
{
    class PhysicalDeviceSelector final
    {
    public:
        PhysicalDeviceSelector( InstanceBundle& instance, std::vector<char const*> const& required_extensions );
        bool select( VkPhysicalDevice device ) const;

    private:
        InstanceBundle& instance_ref_;
        std::vector<char const*> const& required_extensions_{};

        bool check_extension_support( VkPhysicalDevice device ) const;

    };
}


#endif //!PHYSICALDEVICESELECTOR_H
