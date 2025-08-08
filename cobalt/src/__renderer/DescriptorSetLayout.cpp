#include <__renderer/DescriptorSetLayout.h>
#include <__validation/result.h>


namespace cobalt
{
    DescriptorSetLayout::DescriptorSetLayout( DeviceSet const& device, std::span<layout_binding_pair_t const> bindings )
        : device_ref_{ device }
    {
        // The first two fields specify the binding used in the shader and the type of descriptor, which is a uniform buffer object.
        // It is possible for the shader variable to represent an array of uniform buffer objects, and descriptorCount specifies the
        // number of values in the array.
        std::vector<VkDescriptorSetLayoutBinding> layout_bindings( bindings.size( ) );
        std::ranges::transform( bindings, layout_bindings.begin( ),
                                [i = 0u]( auto const& binding_pair ) mutable -> VkDescriptorSetLayoutBinding
                                    {
                                        return {
                                            .binding = i++,
                                            .descriptorCount = 1,
                                            .descriptorType = binding_pair.first,
                                            .stageFlags = binding_pair.second
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
            vkCreateDescriptorSetLayout( device_ref_.logical( ), &layout_info, nullptr, &descriptor_set_layout_ ),
            "Failed to create descriptor set layout!" );
    }


    DescriptorSetLayout::~DescriptorSetLayout( ) noexcept
    {
        vkDestroyDescriptorSetLayout( device_ref_.logical( ), descriptor_set_layout_, nullptr );
    }


    VkDescriptorSetLayout DescriptorSetLayout::handle( ) const noexcept
    {
        return descriptor_set_layout_;
    }

}
