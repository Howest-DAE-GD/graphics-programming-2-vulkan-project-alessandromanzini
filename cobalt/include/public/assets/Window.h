#ifndef WINDOW_H
#define WINDOW_H

#include <__event/multicast_delegate/MulticastDelegate.h>
#include <__memory/Resource.h>

#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan_core.h>
#include <__event/multicast_delegate/Dispatcher.h>


namespace cobalt
{
    class Window final : public memory::Resource
    {
        Dispatcher<VkExtent2D> framebuffer_resize_dispatcher_{};

    public:
        MulticastDelegate<VkExtent2D> on_framebuffer_resize{ framebuffer_resize_dispatcher_ };

        explicit Window( uint32_t width, uint32_t height, char const* title );
        ~Window( ) override;

        Window( Window const& )                = delete;
        Window( Window&& ) noexcept            = delete;
        Window& operator=( Window const& )     = delete;
        Window& operator=( Window&& ) noexcept = delete;

        [[nodiscard]] bool should_close() const;

        [[nodiscard]] std::vector<char const*> get_required_extensions( ) const;
        [[nodiscard]] VkExtent2D extent( ) const;

        void create_surface( VkSurfaceKHR* surface, VkInstance instance ) const;

    private:
        GLFWwindow* window_ptr_{ nullptr };

        static void frame_buffer_size_callback( GLFWwindow* window, int width, int height );

    };

}


#endif //!WINDOW_H
