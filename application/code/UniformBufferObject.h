#ifndef UNIFORMBUFFEROBJECT_H
#define UNIFORMBUFFEROBJECT_H

#include <glm/glm.hpp>


namespace dae
{
    // +---------------------------+
    // | CAMERA                    |
    // +---------------------------+
    struct CameraData
    {
        glm::mat4 model{};
        glm::mat4 view{};
        glm::mat4 proj{};
    };


    // +---------------------------+
    // | LIGHT                     |
    // +---------------------------+
    enum class LightType : uint32_t
    {
        POINT       = 0u,
        DIRECTIONAL = 1u
    };

    struct LightData
    {
        union
        {
            glm::vec4 position;
            glm::vec4 direction;
        } spatial;

        glm::vec4 color{ 1.f };

        union
        {
            glm::vec4 data{ 0.f, 0.f, 0.f, 0.f };
            struct
            {
                float lumen;
                float range;
                LightType type;
                float padding;
            } info;
        } params;
    };

}


#endif //!UNIFORMBUFFEROBJECT_H
