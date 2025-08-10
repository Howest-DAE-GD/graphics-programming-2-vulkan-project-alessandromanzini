#include <log.h>
#include <__render/WriteDescription.h>


namespace cobalt
{
    VkWriteDescriptorSet WriteDescription::create_write_descriptor( VkDescriptorSet const set, uint32_t const frame,
                                                                    uint32_t const dst_binding )
    {
        VkWriteDescriptorSet write_descriptor{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = set,
            .dstBinding = dst_binding,
            .dstArrayElement = 0,
            .descriptorType = desc_type_,
            .descriptorCount = 1
        };

        info_ = generator_( frame );
        switch ( info_.index( ) )
        {
            // VkDescriptorBufferInfo
            case 0:
                write_descriptor.pBufferInfo = &std::get<VkDescriptorBufferInfo>( info_ );
                break;

            // VkDescriptorImageInfo
            case 1:
                write_descriptor.pImageInfo = &std::get<VkDescriptorImageInfo>( info_ );
                break;

            // Fallback
            default:
                log::logerr<WriteDescription>( "create_write_descriptor", "Invalid write descriptor info type!" );
                return {};
        }
        return write_descriptor;
    }
}
