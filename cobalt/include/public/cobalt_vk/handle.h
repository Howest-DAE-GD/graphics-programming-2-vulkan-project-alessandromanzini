#ifndef HANDLE_ALIASES_H
#define HANDLE_ALIASES_H

#include <__memory/Resource.h>
#include <__memory/handle/ResourceHandle.h>


namespace cobalt
{
    template <typename resource_t>
    using DefaultHandle = ResourceHandle<resource_t, memory::Resource>;

    using VkContextHandle = DefaultHandle<class VkContext>;
    using WindowHandle = DefaultHandle<class Window>;

    using BufferHandle = DefaultHandle<class Buffer>;

    using SwapchainHandle = DefaultHandle<class Swapchain>;
    using CommandPoolHandle = DefaultHandle<class CommandPool>;
    using DescriptorAllocatorHandle = DefaultHandle<class DescriptorAllocator>;
    using PipelineLayoutHandle = DefaultHandle<class PipelineLayout>;
    using PipelineHandle = DefaultHandle<class Pipeline>;
    using ImageHandle = DefaultHandle<class Image>;
    using ImageSamplerHandle = DefaultHandle<class ImageSampler>;
    using RendererHandle = DefaultHandle<class Renderer>;
    using ModelHandle = DefaultHandle<class Model>;

}


#endif //!HANDLE_ALIASES_H
