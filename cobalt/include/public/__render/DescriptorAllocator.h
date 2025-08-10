#ifndef DESCRIPTORALLOCATOR_H
#define DESCRIPTORALLOCATOR_H

#include <__memory/Resource.h>

#include <__render/DescriptorSetLayout.h>
#include <__render/WriteDescription.h>

#include <span>


namespace cobalt
{
    class DescriptorAllocator final : public memory::Resource
    {
    public:
        explicit DescriptorAllocator( DeviceSet const&, uint32_t max_frame_in_flight,
                                      std::span<DescriptorSetLayout::layout_binding_pair_t const> bindings );
        ~DescriptorAllocator( ) noexcept override;

        DescriptorAllocator( const DescriptorAllocator& )                = delete;
        DescriptorAllocator( DescriptorAllocator&& ) noexcept            = delete;
        DescriptorAllocator& operator=( const DescriptorAllocator& )     = delete;
        DescriptorAllocator& operator=( DescriptorAllocator&& ) noexcept = delete;

        [[nodiscard]] DescriptorSetLayout const& layout( ) const noexcept;
        [[nodiscard]] VkDescriptorSet set_at( uint32_t frame ) const noexcept;

        void update_sets( std::span<WriteDescription> descriptions ) const;

    private:
        DeviceSet const& device_ref_;
        DescriptorSetLayout const layout_;

        uint32_t const max_frame_in_flight_;

        VkDescriptorPool pool_{ VK_NULL_HANDLE };
        std::vector<VkDescriptorSet> sets_{};

        void create_descriptor_pool( std::span<VkDescriptorType const> desc_types, uint32_t max_frame_in_flight );
        void allocate_descriptor_sets( uint32_t max_frame_in_flight );

    };

}


#endif //!DESCRIPTORALLOCATOR_H
