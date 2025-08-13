#ifndef SURFACEMAP_H
#define SURFACEMAP_H


namespace cobalt
{
    struct SurfaceMap
    {
        int diffuse_color_index{ -1 };
        int normal_map_index{ -1 };
        int metalness_index{ -1 };
        int roughness_index{ -1 };
    };

}


#endif //!SURFACEMAP_H
