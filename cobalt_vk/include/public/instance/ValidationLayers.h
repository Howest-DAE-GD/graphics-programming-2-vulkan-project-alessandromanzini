#ifndef VALIDATIONLAYERS_H
#define VALIDATIONLAYERS_H

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt_vk
{
    class ValidationLayers final
    {
    public:
        using layers_vector_t  = std::vector<const char*>;
        using debug_callback_t = PFN_vkDebugUtilsMessengerCallbackEXT;

        explicit ValidationLayers( layers_vector_t&& layers, debug_callback_t callback );

        void populate_debug_info( VkDebugUtilsMessengerCreateInfoEXT& debugInfo ) const;

        void setup_debug_messenger( VkInstance instance );
        void destroy_debug_messenger( VkInstance instance ) const;

        [[nodiscard]] const layers_vector_t& get_validation_layers( ) const;

    private:
        const layers_vector_t validation_layers_{};

        const debug_callback_t debug_callback_{ nullptr };
        VkDebugUtilsMessengerEXT debug_messenger_{ VK_NULL_HANDLE };

        void check_support() const;

    };

}


#endif //!VALIDATIONLAYERS_H
