#ifndef PIPELINE_H
#define PIPELINE_H

#include <__memory/Resource.h>

#include <vulkan/vulkan_core.h>


namespace cobalt
{
    class DeviceSet;
}

namespace cobalt
{
    class Pipeline : public memory::Resource
    {
    public:
        ~Pipeline( ) noexcept override;

        Pipeline( const Pipeline& )                = delete;
        Pipeline( Pipeline&& ) noexcept            = delete;
        Pipeline& operator=( const Pipeline& )     = delete;
        Pipeline& operator=( Pipeline&& ) noexcept = delete;

        [[nodiscard]] VkPipeline handle( ) const;
        [[nodiscard]] VkPipelineLayout layout( ) const;

    protected:
        DeviceSet const& device_ref_;

        explicit Pipeline( DeviceSet const& );

        void create_layout( VkPipelineLayoutCreateInfo const& );
        void create_handle( VkGraphicsPipelineCreateInfo const& );

    private:
        VkPipelineLayout layout_{ VK_NULL_HANDLE };
        VkPipeline pipeline_{ VK_NULL_HANDLE };

    };

}


#endif //!PIPELINE_H
