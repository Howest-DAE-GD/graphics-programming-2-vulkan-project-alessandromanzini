#ifndef INSTANCEBUNDLE_H
#define INSTANCEBUNDLE_H

#include <vulkan/vulkan_core.h>

#include <vector>


namespace cobalt
{
    class Window;
    class ValidationLayers;
    class InstanceBundle final
    {
    public:
        explicit InstanceBundle( VkApplicationInfo const& app_info, Window const& window,
                                 ValidationLayers const* validation = nullptr );
        ~InstanceBundle( );

        InstanceBundle( InstanceBundle const& )                = delete;
        InstanceBundle( InstanceBundle&& ) noexcept            = delete;
        InstanceBundle& operator=( InstanceBundle const& )     = delete;
        InstanceBundle& operator=( InstanceBundle&& ) noexcept = delete;

        [[nodiscard]] VkInstance get_instance( ) const;
        [[nodiscard]] VkSurfaceKHR get_surface( ) const;

    private:
        VkInstance instance_{ VK_NULL_HANDLE };
        VkSurfaceKHR surface_{ VK_NULL_HANDLE };

        void create_instance( VkApplicationInfo const& app_info, std::vector<char const*> extensions,
                              ValidationLayers const* validation );

    };

}


#endif //!INSTANCEBUNDLE_H
