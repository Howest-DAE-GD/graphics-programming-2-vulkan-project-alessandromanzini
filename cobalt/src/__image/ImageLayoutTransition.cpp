#include <__image/ImageLayoutTransition.h>


namespace cobalt
{
    static std::map<ImageLayoutTransition::layout_pair_t, ImageLayoutTransition::masks_tuple_t> LAYOUT_TO_MASKS_MAP{
        {
            { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL },
            {
                VK_ACCESS_2_NONE, VK_ACCESS_2_TRANSFER_WRITE_BIT,
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_TRANSFER_BIT
            }
        },
        {
            { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
            {
                VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
            }
        },
        {
            { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL },
            {
                VK_ACCESS_2_NONE, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT
            }
        },
        {
                { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL },
                {
                    VK_ACCESS_2_NONE, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                    VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT
                }
        }
    };


    ImageLayoutTransition::ImageLayoutTransition( VkImageLayout const from, VkImageLayout const to ) noexcept
        : ImageLayoutTransition{ from, to, LAYOUT_TO_MASKS_MAP.at( { from, to } ) } { }


    constexpr ImageLayoutTransition::ImageLayoutTransition( VkImageLayout const from, VkImageLayout const to,
                                                            masks_tuple_t const& masks ) noexcept
        : old_layout{ from }
        , new_layout{ to }
        , src_access_mask{ std::get<0>( masks ) }
        , dst_access_mask{ std::get<1>( masks ) }
        , src_stage_mask{ std::get<2>( masks ) }
        , dst_stage_mask{ std::get<3>( masks ) } { }

}
