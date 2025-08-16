#ifndef UNIFORMBUFFEROBJECT_H
#define UNIFORMBUFFEROBJECT_H

#include <glm/glm.hpp>


struct CameraData {
    glm::mat4 model{};
    glm::mat4 view{};
    glm::mat4 proj{};
};


struct CubeMapData
{
    glm::mat4 views[6]{};
    glm::mat4 proj{};
};



#endif //!UNIFORMBUFFEROBJECT_H
