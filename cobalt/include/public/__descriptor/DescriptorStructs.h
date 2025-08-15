#ifndef DESCRIPTORSTRUCTS_H
#define DESCRIPTORSTRUCTS_H

#include <vulkan/vulkan_core.h>


namespace cobalt::descriptor
{
    struct BindingDesc
    {
        VkShaderStageFlags stage_flags{};
        VkDescriptorType descriptor_type{};
        uint32_t descriptor_count{ 1 };
    };

    struct SetAllocRequest
    {
        char const* layout_name{};
        char const* set_name{};
        uint32_t set_count{ 1 };
    };

}


#endif //!DESCRIPTORSTRUCTS_H
