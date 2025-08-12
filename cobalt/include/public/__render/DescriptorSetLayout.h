#ifndef DESCRIPTORSETLAYOUT_H
#define DESCRIPTORSETLAYOUT_H

#include <__context/DeviceSet.h>
#include <__render/LayoutBindingDescription.h>

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
        DescriptorSetLayout( DeviceSet const&, std::span<LayoutBindingDescription const> bindings );
        ~DescriptorSetLayout( ) noexcept override;

        DescriptorSetLayout( const DescriptorSetLayout& )                = delete;
        DescriptorSetLayout( DescriptorSetLayout&& ) noexcept            = delete;
        DescriptorSetLayout& operator=( const DescriptorSetLayout& )     = delete;
        DescriptorSetLayout& operator=( DescriptorSetLayout&& ) noexcept = delete;

        [[nodiscard]] VkDescriptorSetLayout handle( ) const noexcept;
        [[nodiscard]] std::span<LayoutBindingDescription const> descriptor_bindings( ) const noexcept;

    private:
        DeviceSet const& device_ref_;
        VkDescriptorSetLayout desc_set_layout_{ VK_NULL_HANDLE };

        std::vector<LayoutBindingDescription> desc_bindings_{};

    };

}


#endif //!DESCRIPTORSETLAYOUT_H
