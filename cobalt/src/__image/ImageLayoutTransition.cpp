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
            { VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL },
            {
                VK_ACCESS_2_NONE, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT
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
            { VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
            {
                VK_ACCESS_2_TRANSFER_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT,
                VK_PIPELINE_STAGE_2_TRANSFER_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT
            }
        },
    };


    ImageLayoutTransition::ImageLayoutTransition( VkImageLayout const to ) noexcept
        : to_layout{ to } { }


    ImageLayoutTransition& ImageLayoutTransition::from_access( VkAccessFlags2 const flags ) noexcept
    {
        src_access_mask = flags;
        return *this;
    }


    ImageLayoutTransition& ImageLayoutTransition::to_access( VkAccessFlags2 const flags ) noexcept
    {
        dst_access_mask = flags;
        return *this;
    }


    ImageLayoutTransition& ImageLayoutTransition::from_stage( VkPipelineStageFlags2 const flags ) noexcept
    {
        src_stage_mask = flags;
        return *this;
    }


    ImageLayoutTransition& ImageLayoutTransition::to_stage( VkPipelineStageFlags2 const flags ) noexcept
    {
        dst_stage_mask = flags;
        return *this;
    }


    void ImageLayoutTransition::transition_from( VkImageLayout const from ) noexcept
    {
        auto const it = LAYOUT_TO_MASKS_MAP.find( { from, to_layout } );

        if ( it == LAYOUT_TO_MASKS_MAP.end( ) )
        {
            return;
        }

        if ( src_access_mask == UINT64_MAX )
        {
            src_access_mask = std::get<0>( it->second );
        }
        if ( dst_access_mask == UINT64_MAX )
        {
            dst_access_mask = std::get<1>( it->second );
        }
        if ( src_stage_mask == UINT64_MAX )
        {
            src_stage_mask  = std::get<2>( it->second );
        }
        if ( dst_stage_mask == UINT64_MAX )
        {
            dst_stage_mask  = std::get<3>( it->second );
        }
    }

}
