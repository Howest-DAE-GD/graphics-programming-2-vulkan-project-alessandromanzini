#ifndef MATERIAL_H
#define MATERIAL_H


namespace cobalt
{
    struct Material
    {
        int diffuse_color_index{ -1 };
        int specular_color_index{ -1 };
        int normal_map_index{ -1 };
        int metallic_roughness_index{ -1 };
    };

}


#endif //!MATERIAL_H
