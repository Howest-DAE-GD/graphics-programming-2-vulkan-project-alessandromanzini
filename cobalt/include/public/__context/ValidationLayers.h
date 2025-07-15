#ifndef VALIDATIONLAYERS_H
#define VALIDATIONLAYERS_H

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt
{
    class InstanceBundle;
    class ValidationLayers final
    {
    public:
        using debug_callback_t = PFN_vkDebugUtilsMessengerCallbackEXT;

        explicit ValidationLayers( std::vector<char const*> layers, debug_callback_t callback );
        ~ValidationLayers( ) = default;

        ValidationLayers( const ValidationLayers& )                = default;
        ValidationLayers( ValidationLayers&& ) noexcept            = default;
        ValidationLayers& operator=( const ValidationLayers& )     = delete;
        ValidationLayers& operator=( ValidationLayers&& ) noexcept = delete;

        void populate_messanger_debug_info( VkDebugUtilsMessengerCreateInfoEXT& debug_info ) const;

        template <typename create_info_t>
        void populate_create_info( create_info_t& create_info ) const;

        void setup_debug_messenger( InstanceBundle const& instance );
        void destroy_debug_messenger( InstanceBundle const& instance ) const;

        [[nodiscard]] std::vector<char const*> const& get_validation_layers( ) const;

    private:
        std::vector<char const*> const validation_layers_{};

        debug_callback_t const debug_callback_{ nullptr };
        VkDebugUtilsMessengerEXT debug_messenger_{ VK_NULL_HANDLE };

    };


    template <typename create_info_t>
    void ValidationLayers::populate_create_info( create_info_t& create_info ) const
    {
        create_info.enabledLayerCount   = static_cast<uint32_t>( validation_layers_.size( ) );
        create_info.ppEnabledLayerNames = validation_layers_.data( );
    }

}


#endif //!VALIDATIONLAYERS_H
