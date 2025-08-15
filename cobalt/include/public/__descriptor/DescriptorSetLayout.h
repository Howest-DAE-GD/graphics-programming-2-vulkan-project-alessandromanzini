#ifndef DESCRIPTORSETLAYOUT_H
#define DESCRIPTORSETLAYOUT_H

#include <__context/DeviceSet.h>
#include <__descriptor/DescriptorStructs.h>

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
        DescriptorSetLayout( DeviceSet const&, std::span<descriptor::BindingDesc const> bindings, uint32_t total_binding_count );
        ~DescriptorSetLayout( ) noexcept override;

        DescriptorSetLayout( const DescriptorSetLayout& )                = delete;
        DescriptorSetLayout( DescriptorSetLayout&& ) noexcept            = delete;
        DescriptorSetLayout& operator=( const DescriptorSetLayout& )     = delete;
        DescriptorSetLayout& operator=( DescriptorSetLayout&& ) noexcept = delete;

        [[nodiscard]] VkDescriptorSetLayout handle( ) const noexcept;

        [[nodiscard]] std::span<descriptor::BindingDesc const> bindings( ) const noexcept;
        [[nodiscard]] uint32_t total_bindings( ) const noexcept;

    private:
        DeviceSet const& device_ref_;
        VkDescriptorSetLayout desc_set_layout_{ VK_NULL_HANDLE };

        uint32_t const total_binding_count_{ 0 };
        std::vector<descriptor::BindingDesc> desc_bindings_{};

    };

}


#endif //!DESCRIPTORSETLAYOUT_H
