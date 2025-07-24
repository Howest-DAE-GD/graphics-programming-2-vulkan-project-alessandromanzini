#ifndef HANDLE_ALIASES_H
#define HANDLE_ALIASES_H

#include "ResourceHandle.h"


namespace cobalt
{
    template <typename resource_t>
    using DefaultHandle = ResourceHandle<resource_t, class memory::Resource>;

    using VkContextHandle = DefaultHandle<class VkContext>;
    using WindowHandle = DefaultHandle<class Window>;

    using SwapchainHandle = DefaultHandle<class Swapchain>;

    using ModelHandle = DefaultHandle<class Model>;

}


#endif //!HANDLE_ALIASES_H
