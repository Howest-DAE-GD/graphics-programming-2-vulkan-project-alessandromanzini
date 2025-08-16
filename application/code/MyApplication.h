#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include <cobalt_vk/handle.h>

#include <filesystem>
#include <vector>


namespace cobalt
{
    class CommandBuffer;
    class Swapchain;
    class Image;
}
class Camera;


class MyApplication final
{
public:
    MyApplication( );
    ~MyApplication( ) noexcept;

    MyApplication( MyApplication const& )                = delete;
    MyApplication( MyApplication&& ) noexcept            = delete;
    MyApplication& operator=( MyApplication const& )     = delete;
    MyApplication& operator=( MyApplication&& ) noexcept = delete;

    void run( );

private:
    static constexpr uint32_t WIDTH_{ 800u };
    static constexpr uint32_t HEIGHT_{ 600u };

    // In general, we don't want more than 2 frames in flight at a time. That might cause the CPU to get ahead of the GPU.
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT_{ 2u };
    static constexpr uint32_t TEXTURES_COUNT_{ 69u };

    static constexpr std::string_view MODEL_PATH_{ "resources/Sponza.gltf" };
    static constexpr std::string_view SKYBOX_PATH_{ "resources/skybox_4k.hdr" };

    bool running_{ false };

    std::unique_ptr<Camera> camera_ptr_{ nullptr };

    cobalt::WindowHandle window_{};
    cobalt::VkContextHandle context_{};

    cobalt::SwapchainHandle swapchain_{};

    cobalt::CommandPoolHandle command_pool_{};
    cobalt::DescriptorAllocatorHandle descriptor_allocator_{};

    cobalt::PipelineLayoutHandle sampling_pipeline_layout_{};
    cobalt::PipelineLayoutHandle processing_pipeline_layout_{};
    cobalt::PipelineLayoutHandle cubemap_pipeline_layout_{};
    cobalt::PipelineHandle depth_prepass_pipeline_{};
    cobalt::PipelineHandle gbuffer_pass_pipeline_{};
    cobalt::PipelineHandle lighting_pass_pipeline_{};
    cobalt::PipelineHandle post_processing_pass_pipeline_{};
    cobalt::PipelineHandle cubemap_pass_pipeline_{};

    cobalt::RendererHandle renderer_{};

    cobalt::ImageSamplerHandle texture_sampler_{};
    cobalt::ImageCollectionHandle albedo_images_{};
    cobalt::ImageCollectionHandle material_images_{};
    cobalt::ImageCollectionHandle hdr_images_{};
    cobalt::CubeMapImageHandle cubemap_image_{};
    cobalt::ModelHandle model_{};

    std::vector<cobalt::BufferHandle> camera_uniform_buffers_{};

    // .CREATION
    void create_descriptor_allocator( );
    void create_gbuffer_images( );
    void create_post_processing_images( );
    void create_cubemap_image( );
    void create_pipelines( );

    void write_descriptor_sets( );

    // .RENDERING
    void record_command_buffer( cobalt::CommandBuffer const&, cobalt::Swapchain&, uint32_t image_index, uint32_t frame_index );
    void render_to_cubemap( );
    void update_camera_data( uint32_t current_image ) const;

    static void configure_relative_path( );

};


#endif //MYAPPLICATION_H
