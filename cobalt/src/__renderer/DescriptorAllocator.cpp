#include <assert.h>
#include <__renderer/DescriptorAllocator.h>
#include <__renderer/WriteDescription.h>
#include <__validation/result.h>


namespace cobalt
{
    DescriptorAllocator::DescriptorAllocator( DeviceSet const& device,
                                              std::span<DescriptorSetLayout::layout_binding_pair_t const> const bindings )
        : device_ref_{ device }
        , descriptor_set_layout_{ device, bindings } { }


    DescriptorAllocator::~DescriptorAllocator( ) noexcept
    {
        vkDestroyDescriptorPool( device_ref_.logical( ), descriptor_pool_, nullptr );
    }


    DescriptorSetLayout const& DescriptorAllocator::layout( ) const noexcept
    {
        return descriptor_set_layout_;
    }


    VkDescriptorSet DescriptorAllocator::set_at( uint32_t const frame ) const noexcept
    {
        return descriptor_sets_.at( frame );
    }


    void DescriptorAllocator::allocate_pool_and_sets( uint32_t const max_frame_in_flight,
                                                      std::span<VkDescriptorType const> const desc_types )
    {
        create_descriptor_pool( desc_types, max_frame_in_flight );
        allocate_descriptor_sets( max_frame_in_flight );
    }


    void DescriptorAllocator::update_sets( uint32_t const max_frame_in_flight,
                                           std::span<WriteDescription> const descriptions ) const
    {
        for ( uint32_t i{}; i < max_frame_in_flight; i++ )
        {
            std::array<VkWriteDescriptorSet, 16u> descriptor_writes{};
            assert( descriptions.size( ) <= descriptor_writes.size( ) && "too many descriptor writes!" );

            std::ranges::transform( descriptions, descriptor_writes.begin( ),
                                    [this, i, write_offset = 0u]( WriteDescription& desc ) mutable -> VkWriteDescriptorSet
                                        {
                                            return desc.create_write_descriptor( descriptor_sets_[i], i, write_offset++ );
                                        } );

            vkUpdateDescriptorSets( device_ref_.logical( ), static_cast<uint32_t>( descriptions.size( ) ),
                                    descriptor_writes.data( ), 0, nullptr );
        }
    }


    void DescriptorAllocator::create_descriptor_pool( std::span<VkDescriptorType const> desc_types, uint32_t max_frame_in_flight )
    {
        std::vector<VkDescriptorPoolSize> pool_sizes( desc_types.size( ) );
        std::ranges::transform( desc_types, pool_sizes.begin( ),
                                [max_frame_in_flight]( VkDescriptorType const type ) -> VkDescriptorPoolSize
                                    {
                                        return {
                                            .type = type,
                                            .descriptorCount = max_frame_in_flight
                                        };
                                    } );

        VkDescriptorPoolCreateInfo const pool_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,

            .poolSizeCount = static_cast<uint32_t>( pool_sizes.size( ) ),
            .pPoolSizes = pool_sizes.data( ),

            .maxSets = max_frame_in_flight
        };

        validation::throw_on_bad_result(
            vkCreateDescriptorPool( device_ref_.logical( ), &pool_info, nullptr, &descriptor_pool_ ),
            "failed to create descriptor pool!" );
    }


    void DescriptorAllocator::allocate_descriptor_sets( uint32_t const max_frame_in_flight )
    {
        std::vector const layouts{ max_frame_in_flight, descriptor_set_layout_.handle( ) };
        VkDescriptorSetAllocateInfo const alloc_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,

            .descriptorPool = descriptor_pool_,
            .descriptorSetCount = max_frame_in_flight,
            .pSetLayouts = layouts.data( )
        };

        descriptor_sets_.resize( max_frame_in_flight );
        validation::throw_on_bad_result(
            vkAllocateDescriptorSets( device_ref_.logical( ), &alloc_info, descriptor_sets_.data( ) ),
            "failed to allocate descriptor sets!" );
    }

}
