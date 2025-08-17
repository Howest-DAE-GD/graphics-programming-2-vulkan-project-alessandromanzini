#include <__descriptor/DescriptorAllocator.h>

#include <__context/DeviceSet.h>
#include <__descriptor/WriteDescription.h>
#include <__validation/result.h>

#include <algorithm>
#include <cassert>
#include <iterator>


namespace cobalt
{
    DescriptorAllocator::DescriptorAllocator( descriptor::LayoutSpecs specs )
        : device_ref_{ specs.device( ) }
        , layout_map_{ specs.steal_layout( ) }
    {
        create_descriptor_pool( specs );
        allocate_descriptor_sets( specs );
    }


    DescriptorAllocator::~DescriptorAllocator( ) noexcept
    {
        vkDestroyDescriptorPool( device_ref_.logical( ), pool_, nullptr );
    }


    DescriptorSetLayout const& DescriptorAllocator::layout_at( char const* const layout_name ) const noexcept
    {
        assert( layout_map_.contains( layout_name ) && "DescriptorAllocator::layout_at: Layout name not found in the map!" );
        return *layout_map_.at( layout_name );
    }


    DescriptorSet& DescriptorAllocator::set_at( char const* const set_name ) noexcept
    {
        assert( set_map_.contains( set_name ) && "DescriptorAllocator::set_at: Set name not found in the map!" );
        return set_map_.at( set_name );
    }


    void DescriptorAllocator::create_descriptor_pool( descriptor::LayoutSpecs const& specs )
    {
        // 1. Allocate enough space for the pool sizes
        std::vector<VkDescriptorPoolSize> pool_sizes{};
        pool_sizes.reserve( specs.total_binding_count( ) );

        // 2. Iterate through the requests and fill the pool sizes
        for ( descriptor::SetAllocRequest const& req : specs.view_alloc_requests( ) )
        {
            auto const& layout = layout_map_.at( req.layout_name );
            std::ranges::transform(
                layout->bindings( ), std::back_inserter( pool_sizes ),
                [set_count = req.set_count]( descriptor::BindingDesc const& desc ) -> VkDescriptorPoolSize
                    {
                        return {
                            .type = desc.descriptor_type,
                            .descriptorCount = set_count * desc.descriptor_count
                        };
                    } );
        }

        // 3. Create the descriptor pool info
        VkDescriptorPoolCreateInfo const pool_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,

            .maxSets = specs.total_set_count( ),

            .poolSizeCount = static_cast<uint32_t>( pool_sizes.size( ) ),
            .pPoolSizes = pool_sizes.data( ),

            // .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT to release sets individually
        };

        // 4. Create the descriptor pool
        validation::throw_on_bad_result(
            vkCreateDescriptorPool( device_ref_.logical( ), &pool_info, nullptr, &pool_ ),
            "failed to create descriptor pool!" );
    }


    void DescriptorAllocator::allocate_descriptor_sets( descriptor::LayoutSpecs const& specs )
    {
        // 1. Specify layouts in allocation order. We define a layout for each allocation request * count.
        std::vector<VkDescriptorSetLayout> layouts{};
        layouts.reserve( specs.total_set_count( ) );
        for ( descriptor::SetAllocRequest const& req : specs.view_alloc_requests( ) )
        {
            for ( size_t i{}; i < req.set_count; ++i )
            {
                layouts.emplace_back( layout_map_.at( req.layout_name )->handle( ) );
            }
        }

        // 2. Create the allocation info.
        VkDescriptorSetAllocateInfo const alloc_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,

            .descriptorPool = pool_,

            .descriptorSetCount = static_cast<uint32_t>( layouts.size( ) ),
            .pSetLayouts = layouts.data( )
        };

        // 3. Allocate the descriptor sets.
        std::vector<VkDescriptorSet> sets( specs.total_set_count( ) );
        validation::throw_on_bad_result(
            vkAllocateDescriptorSets( device_ref_.logical( ), &alloc_info, sets.data( ) ),
            "failed to allocate descriptor sets!" );

        // 4. Store the sets in the map.
        auto it = sets.begin( );
        for ( auto const& [layout_name, set_name, set_count] : specs.view_alloc_requests( ) )
        {
            auto const& layout = *layout_map_.at( layout_name );
            set_map_.emplace(
                set_name, DescriptorSet{ device_ref_, layout, std::vector<VkDescriptorSet>{ it, std::next( it, set_count ) } } );
            std::advance( it, set_count );
        }
    }

}
