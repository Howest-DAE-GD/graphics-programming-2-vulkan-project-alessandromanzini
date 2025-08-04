#ifndef DESCRIPTORSETLAYOUT_H
#define DESCRIPTORSETLAYOUT_H

#include <__context/DeviceSet.h>


namespace cobalt
{
    class DeviceSet;
}

namespace cobalt
{
    class DescriptorSetLayout final : memory::Resource
    {
    public:
        using layout_binding_pair_t = std::pair<VkDescriptorType, VkShaderStageFlags>;

        DescriptorSetLayout( DeviceSet const&, std::vector<layout_binding_pair_t> const& bindings );
        ~DescriptorSetLayout( ) noexcept override;

        DescriptorSetLayout( const DescriptorSetLayout& )                = delete;
        DescriptorSetLayout( DescriptorSetLayout&& ) noexcept            = delete;
        DescriptorSetLayout& operator=( const DescriptorSetLayout& )     = delete;
        DescriptorSetLayout& operator=( DescriptorSetLayout&& ) noexcept = delete;

        [[nodiscard]] VkDescriptorSetLayout handle( ) const noexcept;

    private:
        DeviceSet const& device_ref_;
        VkDescriptorSetLayout descriptor_set_layout_{ VK_NULL_HANDLE };

    };

}


#endif //!DESCRIPTORSETLAYOUT_H
