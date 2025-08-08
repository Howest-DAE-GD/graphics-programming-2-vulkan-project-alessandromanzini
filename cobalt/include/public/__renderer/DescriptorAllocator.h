#ifndef DESCRIPTORALLOCATOR_H
#define DESCRIPTORALLOCATOR_H

#include <__memory/Resource.h>

#include <__renderer/DescriptorSetLayout.h>
#include <__renderer/WriteDescription.h>

#include <span>


namespace cobalt
{
    class DescriptorAllocator final : memory::Resource
    {
    public:
        explicit DescriptorAllocator( DeviceSet const&, std::span<DescriptorSetLayout::layout_binding_pair_t const> bindings );
        ~DescriptorAllocator( ) noexcept override;

        DescriptorAllocator( const DescriptorAllocator& )                = delete;
        DescriptorAllocator( DescriptorAllocator&& ) noexcept            = delete;
        DescriptorAllocator& operator=( const DescriptorAllocator& )     = delete;
        DescriptorAllocator& operator=( DescriptorAllocator&& ) noexcept = delete;

        [[nodiscard]] DescriptorSetLayout const& layout( ) const noexcept;
        [[nodiscard]] VkDescriptorSet set_at( uint32_t frame ) const noexcept;

        void allocate_pool_and_sets( uint32_t max_frame_in_flight, std::span<VkDescriptorType const> desc_types );
        void update_sets( uint32_t max_frame_in_flight, std::span<WriteDescription> descriptions ) const;

    private:
        DeviceSet const& device_ref_;
        DescriptorSetLayout const descriptor_set_layout_;

        VkDescriptorPool descriptor_pool_{ VK_NULL_HANDLE };
        std::vector<VkDescriptorSet> descriptor_sets_{};

        void create_descriptor_pool( std::span<VkDescriptorType const> desc_types, uint32_t max_frame_in_flight );
        void allocate_descriptor_sets( uint32_t max_frame_in_flight );

    };

}


#endif //!DESCRIPTORALLOCATOR_H
