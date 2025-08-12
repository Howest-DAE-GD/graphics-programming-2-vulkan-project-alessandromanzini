#include <__render/DescriptorSetLayout.h>
#include <__render/LayoutBindingDescription.h>
#include <__validation/result.h>


namespace cobalt
{
    DescriptorSetLayout::DescriptorSetLayout( DeviceSet const& device, std::span<LayoutBindingDescription const> bindings )
        : device_ref_{ device }
        , desc_bindings_{ bindings.begin( ), bindings.end( ) }
    {
        // The first two fields specify the binding used in the shader and the type of descriptor, which is a uniform buffer object.
        // It is possible for the shader variable to represent an array of uniform buffer objects, and descriptorCount specifies the
        // number of values in the array.
        std::vector<VkDescriptorSetLayoutBinding> layout_bindings( desc_bindings_.size( ) );
        std::ranges::transform( desc_bindings_, layout_bindings.begin( ),
                                [i = 0u]( auto const& binding ) mutable -> VkDescriptorSetLayoutBinding
                                    {
                                        return {
                                            .binding = i++,
                                            .descriptorCount = binding.descriptor_count,
                                            .descriptorType = binding.descriptor_type,
                                            .stageFlags = binding.stage_flags
                                        };
                                    } );

        // We need to specify the descriptor set layout during pipeline creation to tell Vulkan which descriptors the shaders will
        // be using.
        VkDescriptorSetLayoutCreateInfo const layout_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>( layout_bindings.size( ) ),
            .pBindings = layout_bindings.data( )
        };

        validation::throw_on_bad_result(
            vkCreateDescriptorSetLayout( device_ref_.logical( ), &layout_info, nullptr, &desc_set_layout_ ),
            "Failed to create descriptor set layout!" );
    }


    DescriptorSetLayout::~DescriptorSetLayout( ) noexcept
    {
        vkDestroyDescriptorSetLayout( device_ref_.logical( ), desc_set_layout_, nullptr );
    }


    VkDescriptorSetLayout DescriptorSetLayout::handle( ) const noexcept
    {
        return desc_set_layout_;
    }


    std::span<LayoutBindingDescription const> DescriptorSetLayout::descriptor_bindings( ) const noexcept
    {
        return desc_bindings_;
    }

}
