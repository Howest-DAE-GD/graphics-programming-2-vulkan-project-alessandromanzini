#ifndef UNIFORMBUFFEROBJECT_H
#define UNIFORMBUFFEROBJECT_H

#include <../../out/cmake-build-debug-x64/_deps/glm-src/glm/glm.hpp>


struct UniformBufferObject {
    glm::mat4 model{};
    glm::mat4 view{};
    glm::mat4 proj{};
};



#endif //!UNIFORMBUFFEROBJECT_H
