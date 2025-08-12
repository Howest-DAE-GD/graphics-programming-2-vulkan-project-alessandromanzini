#ifndef MESH_H
#define MESH_H

#include <cstdint>


namespace cobalt
{
    struct Mesh
    {
        uint32_t index_count{ UINT32_MAX };
        uint32_t index_offset{ UINT32_MAX };
        int32_t vertex_offset{ INT32_MAX };
        uint32_t material_index{ UINT32_MAX };
    };

}


#endif //!MESH_H
