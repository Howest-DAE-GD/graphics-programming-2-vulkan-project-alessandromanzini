#ifndef WINDOW_H
#define WINDOW_H

#include <event/MulticastDelegate.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan_core.h>


namespace cobalt_vk
{
    class Window final
    {
    public:
        Window( uint32_t width, uint32_t height, const char* title );
        ~Window( );

        Window( const Window& )                = delete;
        Window( Window&& ) noexcept            = delete;
        Window& operator=( const Window& )     = delete;
        Window& operator=( Window&& ) noexcept = delete;

        [[nodiscard]] bool get_should_close() const;

        [[nodiscard]] std::vector<const char*> get_required_extensions( ) const;
        [[nodiscard]] std::pair<int, int> get_framebuffer_size( ) const;

        void create_surface( VkSurfaceKHR* surface, VkInstance instance ) const;

    private:
        GLFWwindow* window_ptr_{ nullptr };

        uint32_t window_width_{ 0 };
        uint32_t window_height_{ 0 };

        MulticastDelegate<uint32_t, uint32_t> on_framebuffer_resize_{};

        static void frame_buffer_size_callback( GLFWwindow* window, int width, int height );

    };

}


#endif //!WINDOW_H
