#ifndef DESCRIPTORSET_H
#define DESCRIPTORSET_H

#include <__descriptor/DescriptorSetLayout.h>
#include <__descriptor/WriteDescription.h>

#include <algorithm>
#include <cassert>


namespace cobalt
{
    struct DescriptorSet final
    {
        explicit DescriptorSet( DeviceSet const&, DescriptorSetLayout const&, std::vector<VkDescriptorSet> );

        [[nodiscard]] VkDescriptorSet handle_at( uint32_t index ) const;
        [[nodiscard]] DescriptorSetLayout const& layout( ) const;

        [[nodiscard]] uint32_t parallel_set_count( ) const;

        void update( std::span<WriteDescription> descriptions );
        void update_at( std::span<WriteDescription> descriptions, uint32_t index );

    private:
        DeviceSet const& device_ref_;
        DescriptorSetLayout const& layout_ref_;
        std::vector<VkDescriptorSet> const sets_;

        std::vector<VkWriteDescriptorSet> descriptor_writes_{};

    };


    inline DescriptorSet::DescriptorSet(
        DeviceSet const& device, DescriptorSetLayout const& layout, std::vector<VkDescriptorSet> sets )
        : device_ref_{ device }
        , layout_ref_{ layout }
        , sets_{ std::move( sets ) }
        , descriptor_writes_( layout_ref_.total_bindings( ) ) { }


    inline VkDescriptorSet DescriptorSet::handle_at( uint32_t const index ) const
    {
        return sets_.at( index );
    }


    inline DescriptorSetLayout const& DescriptorSet::layout( ) const
    {
        return layout_ref_;
    }


    inline uint32_t DescriptorSet::parallel_set_count( ) const
    {
        return static_cast<uint32_t>( sets_.size( ) );
    }


    inline void DescriptorSet::update( std::span<WriteDescription> const descriptions )
    {
        for ( uint32_t i{}; i < sets_.size( ); i++ )
        {
            update_at( descriptions, i );
        }
    }


    inline void DescriptorSet::update_at( std::span<WriteDescription> descriptions, uint32_t index )
    {
        std::ranges::transform(
            descriptions, descriptor_writes_.begin( ),
            [this, index, write_offset = 0u]( WriteDescription& desc ) mutable -> VkWriteDescriptorSet
                {
                    return desc.create_write_descriptor( sets_[index], index, write_offset++ );
                } );
        vkUpdateDescriptorSets( device_ref_.logical( ), static_cast<uint32_t>( descriptions.size( ) ),
                                descriptor_writes_.data( ), 0, nullptr );
    }

}


#endif //!DESCRIPTORSET_H
