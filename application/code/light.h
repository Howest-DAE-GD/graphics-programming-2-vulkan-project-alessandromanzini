#ifndef LIGHT_H
#define LIGHT_H

#include "UniformBufferObject.h"


namespace dae::light
{
    void populate_directional_shadow_map_data( LightData& light, glm::vec3 aabb_min, glm::vec3 aabb_max );

}


#endif //!LIGHT_H
