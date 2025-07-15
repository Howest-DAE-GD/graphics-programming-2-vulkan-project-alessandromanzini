#include <assets/Window.h>


namespace cobalt
{
    Window::Window( uint32_t const width, uint32_t const height, char const* title )
        : window_width_{ width }
        , window_height_{ height }
    {
        // Initializes the GLFW library
        glfwInit( );

        // Disable OpenGL context creation
        glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

        // Create the window
        window_ptr_ = glfwCreateWindow( static_cast<int>( window_width_ ), static_cast<int>( window_height_ ),
                                        title, nullptr, nullptr );

        // Set callback pointer to dispatcher
        glfwSetWindowUserPointer( window_ptr_, &on_framebuffer_resize_ );
        glfwSetFramebufferSizeCallback( window_ptr_, frame_buffer_size_callback );
    }


    Window::~Window( )
    {
        glfwDestroyWindow( window_ptr_ );
        glfwTerminate( );
    }


    bool Window::get_should_close( ) const
    {
        constexpr int GLFW_WINDOW_OPEN{ 0 };
        return glfwWindowShouldClose( window_ptr_ ) != GLFW_WINDOW_OPEN;
    }


    std::pair<int, int> Window::get_framebuffer_size( ) const
    {
        int width{}, height{};
        glfwGetFramebufferSize( window_ptr_, &width, &height );
        return std::make_pair( width, height );
    }


    void Window::create_surface( VkSurfaceKHR* surface, VkInstance const instance ) const
    {
        // To manage the output window, we need to include a parameter for the instance, surface creation details,
        // custom allocators and the variable for the surface handle to be stored in.
        // This is technically OS specific, but GLFW provides a cross-platform way to do this.
        // The glfwCreateWindowSurface function performs this operation with a different implementation for each platform.
        glfwCreateWindowSurface( instance, window_ptr_, nullptr, surface );
    }


    std::vector<char const*> Window::get_required_extensions( ) const
    {
        // GLFW has a handy built-in function that returns the extension(s) it needs to do that
        // which we can pass to the struct.
        uint32_t glfw_extension_count{ 0 };
        char const** glfw_extensions{ glfwGetRequiredInstanceExtensions( &glfw_extension_count ) };
        return { glfw_extensions, glfw_extensions + glfw_extension_count };
    }


    void Window::frame_buffer_size_callback( GLFWwindow* window, int const width, int const height )
    {
        auto dispatcher = static_cast<MulticastDelegate<uint32_t, uint32_t> const*>( glfwGetWindowUserPointer( window ) );
        dispatcher->broadcast( static_cast<uint32_t>( width ), static_cast<uint32_t>( height ) );
    }

}
