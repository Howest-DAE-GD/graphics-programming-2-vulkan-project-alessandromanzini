#ifndef VKBUILDER_H
#define VKBUILDER_H

#include <log.h>
#include <__meta/crtp.h>
#include <__meta/vk_function_traits.h>
#include <__validation/result.h>


namespace cobalt::builder
{
    // +---------------------------+
    // | CONCEPTS                  |
    // +---------------------------+
    template <typename builder_t, typename create_fn_t>
    concept VkBuilderWithPopulateCreateInfo =
            requires( typename meta::vk_create_function_traits<create_fn_t>::create_info_t info )
            {
                { static_cast<builder_t*>( nullptr )->populate_create_info( info ) };
            };


    // +---------------------------+
    // | BUILDER CRTP BASE CLASS   |
    // +---------------------------+
    template <typename builder_t, typename create_fn_t>
    class VkBuilder : public ::meta::crtp<builder_t>
    {
        using create_info_t = typename meta::vk_create_function_traits<create_fn_t>::create_info_t;
        using resource_t    = typename meta::vk_create_function_traits<create_fn_t>::resource_t;

        static constexpr std::string_view DEFAULT_ERROR_MESSAGE{ "failed to build resource!" };

    public:
        explicit VkBuilder( VkDevice device, create_fn_t create_fn );
        virtual ~VkBuilder( ) = default;

        VkBuilder( const VkBuilder& )                = delete;
        VkBuilder( VkBuilder&& ) noexcept            = delete;
        VkBuilder& operator=( const VkBuilder& )     = delete;
        VkBuilder& operator=( VkBuilder&& ) noexcept = delete;

        void build( resource_t& resource,
                    create_info_t const* create_info      = nullptr,
                    std::string_view const& error_message = DEFAULT_ERROR_MESSAGE ) const;

    protected:
        [[nodiscard]] VkDevice get_logical_device( ) const;

    private:
        VkDevice const device_{ VK_NULL_HANDLE };
        create_fn_t const create_fn_{ nullptr };

    };


    template <typename builder_t, typename create_fn_t>
    VkBuilder<builder_t, create_fn_t>::VkBuilder( VkDevice const device, create_fn_t create_fn )
        : device_{ device }
        , create_fn_{ create_fn }
    {
        static_assert( VkBuilderWithPopulateCreateInfo<builder_t, create_fn_t>,
                       "builder_t must implement populate_create_info with the correct signature." );
        log::logerr<VkBuilder>( "VkBuilder", "create function must not be null!", create_fn_ == nullptr );
    }


    template <typename builder_t, typename create_fn_t>
    void VkBuilder<builder_t, create_fn_t>::build( resource_t& resource, create_info_t const* create_info,
                                                   std::string_view const& error_message ) const
    {
        auto const& create_info_ref = [this, create_info]( ) -> create_info_t
            {
                if ( create_info == nullptr )
                {
                    create_info_t info{};
                    this->derived_cast( )->populate_create_info( info );
                    return info;
                }
                return *create_info;
            }( );
        validation::throw_on_bad_result( create_fn_( device_, &create_info_ref, nullptr, &resource ), error_message );
    }


    template <typename builder_t, typename create_fn_t>
    VkDevice VkBuilder<builder_t, create_fn_t>::get_logical_device( ) const
    {
        return device_;
    }


}


#endif //!VKBUILDER_H
