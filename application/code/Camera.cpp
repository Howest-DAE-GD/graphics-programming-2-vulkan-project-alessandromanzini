#include "Camera.h"

#include "Timer.h"

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>


namespace dae
{
    Camera::Camera( GLFWwindow& window, VkExtent2D viewport, float fov, float near_plane, float far_plane,
                    glm::vec3 const& origin )
        : window_ptr_{ &window }
        , field_of_view_{ fov }
        , near_plane_{ near_plane }
        , far_plane_{ far_plane }
        , eye_{ origin }
    {
        set_viewport( viewport );
        apply_rotations( );
    }


    void Camera::update( Timer const* timer )
    {
        float const dt = timer->elapsed( );

        // Keyboard Input
        process_keyboard_input( dt );

        // Mouse Input
        if ( std::pair<double, double> mouse_pos;
            calculate_delta_mouse_movement( mouse_pos.first, mouse_pos.second ) &&
            glfwGetMouseButton( window_ptr_, GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS )
        {
            process_mouse_input( mouse_pos.first, mouse_pos.second, dt );
        }
    }


    glm::mat4x4 Camera::camera_to_world( ) const
    {
        return glm::lookAt( eye_, eye_ + forward_, up_ );
    }


    glm::mat4x4 Camera::projection( ) const
    {
        auto proj = glm::perspective( field_of_view_, aspect_ratio_, near_plane_, far_plane_ );
        proj[1][1] *= -1;
        return proj;
    }


    glm::vec3 const& Camera::view_direction( ) const noexcept
    {
        return forward_;
    }


    glm::vec3 const& Camera::eye( ) const noexcept
    {
        return eye_;
    }


    void Camera::set_viewport( VkExtent2D const extent ) noexcept
    {
        aspect_ratio_ = static_cast<float>( extent.width ) / extent.height;
    }


    void Camera::set_pitch( double const pitch ) noexcept
    {
        constexpr auto vertical_thresh = static_cast<double>( glm::radians( 85.f ) );

        pitch_ = std::clamp( pitch, -vertical_thresh, vertical_thresh );
        apply_rotations( );
    }


    void Camera::set_yaw( double const yaw ) noexcept
    {
        yaw_ = yaw;
        apply_rotations( );
    }


    void Camera::apply_rotations( )
    {
        forward_ = normalize( glm::vec3{
            sin( yaw_ ) * cos( glm::radians( pitch_ ) ),
            sin( pitch_ ),
            cos( yaw_ ) * cos( glm::radians( pitch_ ) )
        } );
        right_ = normalize( cross( forward_, WORLD_UP_ ) );
        up_    = normalize( cross( right_, forward_ ) );
    }


    void Camera::process_keyboard_input( float const dt )
    {
        float const speed{ dt * MOVEMENT_SPEED_ * ( glfwGetKey( window_ptr_, GLFW_KEY_LEFT_SHIFT ) ? SPRINT_MODIFIER_ : 1.f ) };

        if ( glfwGetKey( window_ptr_, GLFW_KEY_W ) )
        {
            eye_ += forward_ * speed;
        }
        if ( glfwGetKey( window_ptr_, GLFW_KEY_S ) )
        {
            eye_ -= forward_ * speed;
        }
        if ( glfwGetKey( window_ptr_, GLFW_KEY_A ) )
        {
            eye_ -= right_ * speed;
        }
        if ( glfwGetKey( window_ptr_, GLFW_KEY_D ) )
        {
            eye_ += right_ * speed;
        }
        if ( glfwGetKey( window_ptr_, GLFW_KEY_E ) )
        {
            eye_ += WORLD_UP_ * speed;
        }
        if ( glfwGetKey( window_ptr_, GLFW_KEY_Q ) )
        {
            eye_ -= WORLD_UP_ * speed;
        }
    }


    void Camera::process_mouse_input( double const yaw, double const pitch, float const dt )
    {
        float const speed{ dt * CAMERA_ROTATION_SPEED_ * INVERT_CAMERA_AXIS_ };

        set_pitch( pitch_ + pitch * speed );
        set_yaw( yaw_ + yaw * speed );
    }


    bool Camera::calculate_delta_mouse_movement( double& x, double& y )
    {
        double mouse_x{};
        double mouse_y{};
        glfwGetCursorPos( window_ptr_, &mouse_x, &mouse_y );

        if ( last_mouse_x_ == DBL_MIN || last_mouse_y_ == DBL_MIN )
        {
            x = 0.0;
            y = 0.0;
        }
        else
        {
            x = mouse_x - last_mouse_x_;
            y = mouse_y - last_mouse_y_;
        }

        last_mouse_x_ = mouse_x;
        last_mouse_y_ = mouse_y;

        return not( x > -DBL_EPSILON && x < DBL_EPSILON ) || not( y > -DBL_EPSILON && y < DBL_EPSILON );
    }

}
