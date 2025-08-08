#include <__context/Window.h>


namespace cobalt
{
    Window::Window( uint32_t const width, uint32_t const height, char const* title )
    {
        // Initializes the GLFW library
        glfwInit( );

        // Disable OpenGL context creation
        glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

        // Create the window
        window_ptr_ = glfwCreateWindow( static_cast<int>( width ), static_cast<int>( height ),
                                        title, nullptr, nullptr );

        // Set callback pointer to dispatcher
        glfwSetWindowUserPointer( window_ptr_, &framebuffer_resize_dispatcher_ );
        glfwSetFramebufferSizeCallback( window_ptr_, framebuffer_resize_callback );
    }


    Window::~Window( )
    {
        glfwDestroyWindow( window_ptr_ );
        glfwTerminate( );
    }


    bool Window::should_close( ) const
    {
        constexpr int GLFW_WINDOW_OPEN{ 0 };
        return glfwWindowShouldClose( window_ptr_ ) != GLFW_WINDOW_OPEN;
    }


    bool Window::is_minimized( ) const
    {
        auto const [width, height] = extent( );
        return width == 0 || height == 0;
    }


    VkExtent2D Window::extent( ) const
    {
        int width{}; int height{};
        glfwGetFramebufferSize( window_ptr_, &width, &height );
        return { static_cast<uint32_t>( width ), static_cast<uint32_t>( height ) };
    }


    void Window::create_surface( VkSurfaceKHR* surface, VkInstance const instance ) const
    {
        // To manage the output window, we need to include a parameter for the instance, surface creation details,
        // custom allocators and the variable for the surface handle to be stored in.
        // This is technically OS specific, but GLFW provides a cross-platform way to do this.
        // The glfwCreateWindowSurface function performs this operation with a different implementation for each platform.
        glfwCreateWindowSurface( instance, window_ptr_, nullptr, surface );
    }


    void Window::force_framebuffer_resize( ) const
    {
        framebuffer_resize_dispatcher_.broadcast( extent( ) );
    }


    std::vector<char const*> Window::get_required_extensions( ) const
    {
        // GLFW has a handy built-in function that returns the extension(s) it needs to do that
        // which we can pass to the struct.
        uint32_t glfw_extension_count{ 0 };
        char const** glfw_extensions{ glfwGetRequiredInstanceExtensions( &glfw_extension_count ) };
        return { glfw_extensions, glfw_extensions + glfw_extension_count };
    }


    void Window::framebuffer_resize_callback( GLFWwindow* window, int const width, int const height )
    {
        auto const dispatcher =
                static_cast<decltype(framebuffer_resize_dispatcher_) const*>( glfwGetWindowUserPointer( window ) );
        dispatcher->broadcast( { static_cast<uint32_t>( width ), static_cast<uint32_t>( height ) } );
    }

}
