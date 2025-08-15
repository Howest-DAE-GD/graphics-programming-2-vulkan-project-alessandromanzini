#ifndef PIPELINELAYOUT_H
#define PIPELINELAYOUT_H

#include <__memory/Resource.h>

#include <vulkan/vulkan_core.h>

#include <span>


namespace cobalt
{
    class DeviceSet;
    class DescriptorSetLayout;
}

namespace cobalt
{
    class PipelineLayout final : public memory::Resource
    {
    public:
        PipelineLayout( DeviceSet const&, std::span<DescriptorSetLayout const* const> desc_layouts,
                        std::span<VkPushConstantRange const> push_constant_ranges = {} );
        ~PipelineLayout( ) noexcept override;

        PipelineLayout( const PipelineLayout& )                = delete;
        PipelineLayout( PipelineLayout&& ) noexcept            = delete;
        PipelineLayout& operator=( const PipelineLayout& )     = delete;
        PipelineLayout& operator=( PipelineLayout&& ) noexcept = delete;

        [[nodiscard]] VkPipelineLayout handle( ) const noexcept;
        [[nodiscard]] std::span<DescriptorSetLayout const* const> descriptor_layouts( ) const noexcept;

    private:
        DeviceSet const& device_ref_;

        std::vector<DescriptorSetLayout const*> descriptor_layouts_{};

        VkPipelineLayout layout_{ VK_NULL_HANDLE };

    };

}


#endif //!PIPELINELAYOUT_H
