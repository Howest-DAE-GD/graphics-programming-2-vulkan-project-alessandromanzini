
// STRUCTS
struct SurfaceMap
{
    uint base_color_id;
    uint normal_id;
    uint metalness_id;
    uint roughness_id;
    uint ao_id;
    uint padding[3];
};


// BINDINNGS
layout ( push_constant ) uniform SurfaceDescription { uint surface_id; } SD;
layout ( set = 0, binding = 3 ) readonly buffer SurfaceBufferData { SurfaceMap maps[]; } surface_buffer;


// FUNCTIONS
bool is_surface_map_valid( SurfaceMap map )
{
    const uint uint_max = 0xFFFFFFFFu;
    return map.base_color_id != uint_max && map.normal_id != uint_max && map.metalness_id != uint_max &&
           map.roughness_id != uint_max;
}