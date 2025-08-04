#ifndef BINDINGDESCRIPTION_H
#define BINDINGDESCRIPTION_H

#include <vector>
#include <vulkan/vulkan_core.h>


namespace cobalt
{
    struct BindingDescription
    {
        VkVertexInputBindingDescription vertex_desc{ 0, 0, VK_VERTEX_INPUT_RATE_MAX_ENUM };
        std::vector<VkVertexInputAttributeDescription> attributes_desc{};
    };

}


#endif //!BINDINGDESCRIPTION_H
