#ifndef SURFACEMAP_H
#define SURFACEMAP_H

#include <glm/vec4.hpp>

#include <cstdint>


namespace cobalt
{
    struct SurfaceMap
    {
        union BaseMaterialUnion
        {
            glm::uvec4 indices{ UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
            struct BaseMaterialIndices
            {
                uint32_t base_color_index;
                uint32_t normal_map_index;
                uint32_t metalness_index;
                uint32_t roughness_index;
            } value;
        } base;

        union ExtraMaterialUnion
        {
            glm::uvec4 indices{ UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX };
            struct ExtraMaterialIndices
            {
                uint32_t ao_index;
                uint32_t padding[3];
            } value;
        } extra;
    };

}


#endif //!SURFACEMAP_H
