#ifndef WRITEDESCRIPTION_H
#define WRITEDESCRIPTION_H

#include <vulkan/vulkan_core.h>

#include <functional>
#include <variant>


namespace cobalt
{
    namespace internal
    {
        using write_desc_info_variant_t = std::variant<VkDescriptorBufferInfo, VkDescriptorImageInfo>;
    }


    class WriteDescription final
    {
    public:
        template <typename gen_fn_t>
        WriteDescription( VkDescriptorType type, gen_fn_t&& gen );

        [[nodiscard]] VkWriteDescriptorSet create_write_descriptor( VkDescriptorSet set, uint32_t frame, uint32_t dst_binding );

    private:
        std::function<internal::write_desc_info_variant_t( uint32_t )> const generator_;
        VkDescriptorType const desc_type_;

        internal::write_desc_info_variant_t info_{};

    };


    template <typename gen_fn_t>
    WriteDescription::WriteDescription( VkDescriptorType const type, gen_fn_t&& gen )
        : generator_{ std::move( gen ) }
        , desc_type_{ type } { }

}


#endif //!WRITEDESCRIPTION_H
