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

        ImageLayoutTransition( VkImageLayout to ) noexcept;

        ImageLayoutTransition& from_access( VkAccessFlags ) noexcept;
        ImageLayoutTransition& to_access( VkAccessFlags ) noexcept;

        ImageLayoutTransition& from_stage( VkAccessFlags ) noexcept;
        ImageLayoutTransition& to_stage( VkAccessFlags ) noexcept;

        void transition_from( VkImageLayout const from ) noexcept;

        VkImageLayout const to_layout;

        VkAccessFlags2 src_access_mask{ UINT64_MAX };
        VkAccessFlags2 dst_access_mask{ UINT64_MAX };
        VkPipelineStageFlags2 src_stage_mask{ UINT64_MAX };
        VkPipelineStageFlags2 dst_stage_mask{ UINT64_MAX };

    };

}


#endif //!IMAGELAYOUTTRANSITION_H
