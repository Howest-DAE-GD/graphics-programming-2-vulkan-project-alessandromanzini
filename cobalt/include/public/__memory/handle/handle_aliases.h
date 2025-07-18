#ifndef HANDLE_ALIASES_H
#define HANDLE_ALIASES_H

#include "ResourceHandle.h"


namespace cobalt
{
    namespace memory
    {
        class Resource;
    }
    class VkContext;
    class Window;
    class Model;

    template <typename resource_t>
    using DefaultHandle = ResourceHandle<resource_t, memory::Resource>;

    using VkContextHandle = DefaultHandle<VkContext>;
    using WindowHandle = DefaultHandle<Window>;

    using ModelHandle = DefaultHandle<Model>;

}


#endif //!HANDLE_ALIASES_H
