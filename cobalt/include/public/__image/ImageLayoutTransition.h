#ifndef IMAGELAYOUTTRANSITION_H
#define IMAGELAYOUTTRANSITION_H

#include <vulkan/vulkan_core.h>

#include <map>


namespace cobalt
{
    struct ImageLayoutTransition final
    {
        using layout_pair_t = std::pair<VkImageLayout, VkImageLayout>;
        using masks_tuple_t = std::tuple<VkPipelineStageFlags2, VkPipelineStageFlags2, VkAccessFlags2, VkAccessFlags2>;

        ImageLayoutTransition( VkImageLayout from, VkImageLayout to ) noexcept;
        constexpr ImageLayoutTransition( VkImageLayout from, VkImageLayout to, masks_tuple_t const& ) noexcept;

        VkImageLayout const old_layout;
        VkImageLayout const new_layout;

        VkAccessFlags2 const src_access_mask;
        VkAccessFlags2 const dst_access_mask;
        VkPipelineStageFlags2 const src_stage_mask;
        VkPipelineStageFlags2 const dst_stage_mask;

    };

}


#endif //!IMAGELAYOUTTRANSITION_H
