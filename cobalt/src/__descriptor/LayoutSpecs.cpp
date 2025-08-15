#include <__descriptor/LayoutSpecs.h>

#include <cassert>
#include <numeric>


namespace cobalt::descriptor
{
    LayoutSpecs::LayoutSpecs( DeviceSet const& device )
        : device_ref_{ device } { }


    LayoutSpecs& LayoutSpecs::define( char const* const layout_name, std::initializer_list<BindingDesc const> bindings ) &
    {
        assert( not layouts_.contains( layout_name ) && "descriptor set layout with this name already exists!" );

        uint32_t const binding_count = std::reduce(
            bindings.begin( ), bindings.end( ), 0u,
            []( uint32_t const acc, BindingDesc const& desc ) { return acc + desc.descriptor_count; } );

        total_binding_count_ += binding_count;
        layouts_.insert( { layout_name, std::make_unique<DescriptorSetLayout>( device_ref_, bindings, binding_count ) } );
        return *this;
    }


    LayoutSpecs&& LayoutSpecs::define( char const* const layout_name, std::initializer_list<BindingDesc const> bindings ) &&
    {
        return std::move( define( layout_name, std::move( bindings ) ) );
    }


    LayoutSpecs& LayoutSpecs::alloc( char const* const set_name, char const* const layout_name, uint32_t const set_count ) &
    {
        total_set_count_ += static_cast<uint32_t>( set_count );
        alloc_requests_.emplace_back( SetAllocRequest{
            .layout_name = layout_name,
            .set_name = set_name,
            .set_count = set_count,
        } );
        return *this;
    }


    LayoutSpecs&& LayoutSpecs::alloc( char const* const set_name, char const* const layout_name, uint32_t const set_count ) &&
    {
        return std::move( alloc( set_name, layout_name, set_count ) );
    }


    LayoutSpecs::layout_map_t&& LayoutSpecs::steal_layout( )
    {
        return std::move( layouts_ );
    }


    std::span<SetAllocRequest const> LayoutSpecs::view_alloc_requests( ) const
    {
        return std::span{ alloc_requests_ };
    }


    uint32_t LayoutSpecs::total_binding_count( ) const noexcept
    {
        return total_binding_count_;
    }


    uint32_t LayoutSpecs::total_set_count( ) const noexcept
    {
        return total_set_count_;
    }


    DeviceSet const& LayoutSpecs::device( ) const noexcept
    {
        return device_ref_;
    }

}
