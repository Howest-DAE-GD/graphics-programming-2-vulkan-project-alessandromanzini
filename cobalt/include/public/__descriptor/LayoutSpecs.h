#ifndef LAYOUTSPECS_H
#define LAYOUTSPECS_H

#include <__descriptor/DescriptorSetLayout.h>
#include <__meta/cstr_comparator.h>

#include <map>
#include <vector>


namespace cobalt::descriptor
{
    class LayoutSpecs final
    {
    public:
        using layout_map_t = std::map<char const*, std::unique_ptr<DescriptorSetLayout>, meta::c_str_less>;

        explicit LayoutSpecs( DeviceSet const& );

        LayoutSpecs& define( char const* layout_name, std::initializer_list<BindingDesc const> bindings ) &;
        LayoutSpecs&& define( char const* layout_name, std::initializer_list<BindingDesc const> bindings ) &&;

        LayoutSpecs& alloc( char const* set_name, char const* layout_name, uint32_t set_count = 1u ) &;
        LayoutSpecs&& alloc( char const* set_name, char const* layout_name, uint32_t set_count = 1u ) &&;

        [[nodiscard]] layout_map_t&& steal_layout( );
        [[nodiscard]] std::span<SetAllocRequest const> view_alloc_requests( ) const;
        [[nodiscard]] uint32_t total_binding_count( ) const noexcept;
        [[nodiscard]] uint32_t total_set_count( ) const noexcept;

        [[nodiscard]] DeviceSet const& device( ) const noexcept;

    private:
        DeviceSet const& device_ref_;

        layout_map_t layouts_{};
        std::vector<SetAllocRequest> alloc_requests_{};

        uint32_t total_binding_count_{ 0u };
        uint32_t total_set_count_{ 0u };

    };

}


#endif //!LAYOUTSPECS_H
