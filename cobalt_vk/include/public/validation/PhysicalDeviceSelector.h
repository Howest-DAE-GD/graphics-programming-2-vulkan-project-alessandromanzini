#ifndef PHYSICALDEVICESELECTOR_H
#define PHYSICALDEVICESELECTOR_H

#include <instance/VulkanInstance.h>

#include <vector>


namespace cobalt_vk::validation
{
    class PhysicalDeviceSelector final
    {
    public:
        PhysicalDeviceSelector( VulkanInstance& instance, std::vector<const char*> requiredExtensions );
        bool select( VkPhysicalDevice device ) const;

    private:
        VulkanInstance& instance_ref_;
        std::vector<const char*> required_extensions_{};

        bool check_extension_support( VkPhysicalDevice device ) const;

    };
}


#endif //!PHYSICALDEVICESELECTOR_H
