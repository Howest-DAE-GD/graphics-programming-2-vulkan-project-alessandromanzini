#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>


namespace dae
{
    class Timer;
}

namespace dae
{
    // todo: use dirty flagging
    class Camera final
    {
    public:
        explicit Camera( GLFWwindow& window, VkExtent2D viewport, float fov, float near_plane, float far_plane,
                         glm::vec3 const& origin = { 0.f, 0.f, 0.f } );

        void update( Timer const* timer );

        [[nodiscard]] glm::mat4x4 camera_to_world( ) const;
        [[nodiscard]] glm::mat4x4 projection( ) const;
        [[nodiscard]] glm::vec3 const& view_direction( ) const noexcept;
        [[nodiscard]] glm::vec3 const& eye( ) const noexcept;

        void set_viewport( VkExtent2D ) noexcept;
        void set_pitch( double ) noexcept;
        void set_yaw( double ) noexcept;

    private:
        static constexpr float MOVEMENT_SPEED_{ 2.f };
        static constexpr float SPRINT_MODIFIER_{ 4.f };
        static constexpr float CAMERA_ROTATION_SPEED_{ 0.4f };
        static constexpr int8_t INVERT_CAMERA_AXIS_{ -1 };

        static constexpr glm::vec3 WORLD_UP_{ 0.f, 1.f, 0.f };

        GLFWwindow* const window_ptr_;

        float const field_of_view_{ 45.0f };
        float const near_plane_{ 0.01f };
        float const far_plane_{ 50.0f };
        float aspect_ratio_{};

        double pitch_{ 0.f };
        double yaw_{ 0.f };

        glm::vec3 eye_{};
        glm::vec3 forward_{};
        glm::vec3 up_{};
        glm::vec3 right_{};

        double last_mouse_x_{ DBL_MIN };
        double last_mouse_y_{ DBL_MIN };

        void apply_rotations( );
        void process_keyboard_input( float dt );
        void process_mouse_input( double yaw, double pitch, float dt );

        bool calculate_delta_mouse_movement( double& x, double& y );

    };

}


#endif //!CAMERA_H
