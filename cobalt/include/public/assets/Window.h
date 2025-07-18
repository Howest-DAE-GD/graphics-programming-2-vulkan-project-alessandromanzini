#ifndef WINDOW_H
#define WINDOW_H

#include <event/MulticastDelegate.h>
#include <__memory/Resource.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan_core.h>


namespace cobalt
{
    class Window final : public memory::Resource
    {
    public:
        explicit Window( uint32_t width, uint32_t height, char const* title );
        ~Window( ) override;

        Window( Window const& )                = delete;
        Window( Window&& ) noexcept            = delete;
        Window& operator=( Window const& )     = delete;
        Window& operator=( Window&& ) noexcept = delete;

        [[nodiscard]] bool get_should_close() const;

        [[nodiscard]] std::vector<char const*> get_required_extensions( ) const;
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
