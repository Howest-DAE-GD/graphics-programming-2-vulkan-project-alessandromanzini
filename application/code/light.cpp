#include "light.h"

#include <glm/gtc/matrix_transform.hpp>


namespace dae::light
{
    void populate_directional_shadow_map_data( LightData& light, glm::vec3 const aabb_min, glm::vec3 const aabb_max )
    {
        // calculate center off provided AABB
        glm::vec3 const scene_center    = ( aabb_min + aabb_max ) * 0.5f;
        glm::vec3 const light_direction = normalize( light.spatial.direction );

        // create 8 AABB corners for light projection
        std::array const corners{
            glm::vec3{ aabb_min.x, aabb_min.y, aabb_min.z },
            glm::vec3{ aabb_max.x, aabb_min.y, aabb_min.z },
            glm::vec3{ aabb_min.x, aabb_max.y, aabb_min.z },
            glm::vec3{ aabb_max.x, aabb_max.y, aabb_min.z },
            glm::vec3{ aabb_min.x, aabb_min.y, aabb_max.z },
            glm::vec3{ aabb_max.x, aabb_min.y, aabb_max.z },
            glm::vec3{ aabb_min.x, aabb_max.y, aabb_max.z },
            glm::vec3{ aabb_max.x, aabb_max.y, aabb_max.z }
        };

        // project AABB corners onto light direction
        auto const [min_proj, max_proj] = [&]
            {
                float min = FLT_MAX;
                float max = -FLT_MAX;
                for ( glm::vec3 const& corner : corners )
                {
                    float const proj = dot( light_direction, corner );

                    min = std::min( min, proj );
                    max = std::max( max, proj );
                }
                return std::make_pair( min, max );
            }( );

        // calculate distance and position
        float const distance = max_proj - dot( scene_center, light_direction );

        glm::vec3 const light_position = scene_center - light_direction * distance;

        // calculate safe up vector
        glm::vec3 const up = std::abs( dot( light_direction, glm::vec3{ 0.f, 1.f, 0.f } ) ) > 0.99f
                                 ? glm::vec3{ 0.f, 0.f, 1.f }
                                 : glm::vec3{ 0.f, 1.f, 0.f };

        // use light position with center to generate view matrix - inverted Y-axis for up
        light.view = lookAt( light_position, scene_center, up );

        // now go over the AABB corners again to find the min and max extents
        auto const [min_light_space, max_light_space] = [&]
            {
                glm::vec3 min{ FLT_MAX };
                glm::vec3 max{ -FLT_MAX };
                for ( glm::vec3 const& corner : corners )
                {
                    glm::vec3 const transformed_corner{ light.view * glm::vec4{ corner, 1.f } };
                    min = glm::min( min, transformed_corner );
                    max = glm::max( max, transformed_corner );
                }
                return std::make_pair( min, max );
            }( );

        // with min-max create the orthographic projection matrix
        float const far_z = max_light_space.z - min_light_space.z;

        light.proj = glm::ortho( min_light_space.x, max_light_space.x, min_light_space.y, max_light_space.y, 0.f, far_z );
    }

}
