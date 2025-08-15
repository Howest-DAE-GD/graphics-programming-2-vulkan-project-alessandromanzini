#ifndef PIPELINE_H
#define PIPELINE_H

#include <__memory/Resource.h>

#include <__descriptor/DescriptorSet.h>
#include <__pipeline/PipelineLayout.h>

#include <vulkan/vulkan_core.h>


namespace cobalt
{
    class DeviceSet;
}

namespace cobalt
{
    struct PipelineCreateInfo
    {
        std::span<DescriptorSet const* const> descriptor_sets;
        VkPipelineBindPoint bind_point;
        VkGraphicsPipelineCreateInfo create_info;
    };


    class Pipeline final : public memory::Resource
    {
    public:
        explicit Pipeline( DeviceSet const&, PipelineLayout const&, PipelineCreateInfo const& );
        ~Pipeline( ) noexcept override;

        Pipeline( Pipeline&& ) noexcept;
        Pipeline( const Pipeline& )                = delete;
        Pipeline& operator=( const Pipeline& )     = delete;
        Pipeline& operator=( Pipeline&& ) noexcept = delete;

        [[nodiscard]] VkPipeline handle( ) const;
        [[nodiscard]] PipelineLayout const& layout( ) const;

        [[nodiscard]] VkPipelineBindPoint bind_point( ) const;
        [[nodiscard]] std::span<DescriptorSet const* const> descriptor_sets( ) const;

    private:
        DeviceSet const& device_ref_;
        PipelineLayout const& layout_ref_;

        VkPipelineBindPoint const bind_point_;
        std::vector<DescriptorSet const*> const descriptor_sets_{};

        VkPipeline pipeline_{ VK_NULL_HANDLE };

    };

}


#endif //!PIPELINE_H
