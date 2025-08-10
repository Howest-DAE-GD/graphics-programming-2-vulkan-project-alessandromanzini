#ifndef DESCRIPTORSETLAYOUT_H
#define DESCRIPTORSETLAYOUT_H

#include <__context/DeviceSet.h>

#include <span>


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

        DescriptorSetLayout( DeviceSet const&, std::span<layout_binding_pair_t const> bindings );
        ~DescriptorSetLayout( ) noexcept override;

        DescriptorSetLayout( const DescriptorSetLayout& )                = delete;
        DescriptorSetLayout( DescriptorSetLayout&& ) noexcept            = delete;
        DescriptorSetLayout& operator=( const DescriptorSetLayout& )     = delete;
        DescriptorSetLayout& operator=( DescriptorSetLayout&& ) noexcept = delete;

        [[nodiscard]] VkDescriptorSetLayout handle( ) const noexcept;
        [[nodiscard]] std::span<VkDescriptorType const> descriptor_types( ) const noexcept;

    private:
        DeviceSet const& device_ref_;

        std::vector<VkDescriptorType> desc_types_{};
        VkDescriptorSetLayout desc_set_layout_{ VK_NULL_HANDLE };

    };

}


#endif //!DESCRIPTORSETLAYOUT_H
