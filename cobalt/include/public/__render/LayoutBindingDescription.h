#ifndef LAYOUTBINDINGDESCRIPTION_H
#define LAYOUTBINDINGDESCRIPTION_H

#include <vulkan/vulkan_core.h>


namespace cobalt
{
    struct LayoutBindingDescription
    {
        VkShaderStageFlags stage_flags{};
        VkDescriptorType descriptor_type{};
        uint32_t descriptor_count{ 1 };

    };

}


#endif //!LAYOUTBINDINGDESCRIPTION_H
