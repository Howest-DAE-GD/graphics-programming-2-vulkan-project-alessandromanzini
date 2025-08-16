#ifndef DESCRIPTORALLOCATOR_H
#define DESCRIPTORALLOCATOR_H

#include <__memory/Resource.h>

#include <__descriptor/DescriptorSet.h>
#include <__descriptor/DescriptorSetLayout.h>
#include <__descriptor/LayoutSpecs.h>
#include <__meta/cstr_comparator.h>

#include <span>


namespace cobalt
{
    class DescriptorAllocator final : public memory::Resource
    {
    public:
        explicit DescriptorAllocator( descriptor::LayoutSpecs );
        ~DescriptorAllocator( ) noexcept override;

        DescriptorAllocator( const DescriptorAllocator& )                = delete;
        DescriptorAllocator( DescriptorAllocator&& ) noexcept            = delete;
        DescriptorAllocator& operator=( const DescriptorAllocator& )     = delete;
        DescriptorAllocator& operator=( DescriptorAllocator&& ) noexcept = delete;

        [[nodiscard]] DescriptorSetLayout const& layout_at( char const* layout_name ) const noexcept;
        [[nodiscard]] DescriptorSet& set_at( char const* set_name ) noexcept;

    private:
        DeviceSet const& device_ref_;

        descriptor::LayoutSpecs::layout_map_t const layout_map_;
        std::map<char const*, DescriptorSet, meta::c_str_less> set_map_{};

        VkDescriptorPool pool_{ VK_NULL_HANDLE };

        void create_descriptor_pool( descriptor::LayoutSpecs const& );
        void allocate_descriptor_sets( descriptor::LayoutSpecs const& );

    };

}


#endif //!DESCRIPTORALLOCATOR_H
