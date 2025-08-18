#ifndef MYAPPLICATION_H
#define MYAPPLICATION_H

#include "UniformBufferObject.h"

#include <cobalt_vk/handle.h>
#include <vulkan/vulkan_core.h>

#include <filesystem>
#include <array>


namespace cobalt::shader
{
    class ShaderModule;
}

namespace cobalt
{
    class CommandBuffer;
    class Swapchain;
    class Image;
}

namespace dae
{
    class Camera;
}

namespace dae
{
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
        static constexpr uint32_t TEXTURE_COUNT_{ 69u };

        static constexpr uint32_t SHADOW_MAP_SIZE_{ 1024u };

        static constexpr std::string_view MODEL_PATH_{ "resources/Sponza.gltf" };
        static constexpr std::string_view SKYBOX_PATH_{ "resources/skybox_4k.hdr" };

        static constexpr uint32_t LIGHT_COUNT_{ 1u };
        std::array<LightData, LIGHT_COUNT_> lights_{
            LightData{
                .spatial = { .direction = glm::vec4{ 0.f, -1.f, -.1f, 0.f } },
                .params = {
                    .info = { .kelvin = 12'000.f, .lumen = 800.f, .type = LightType::DIRECTIONAL }
                },
            },
        };

        bool running_{ false };

        std::unique_ptr<Camera> camera_ptr_{ nullptr };

        cobalt::WindowHandle window_{};
        cobalt::VkContextHandle context_{};

        cobalt::SwapchainHandle swapchain_{};

        cobalt::CommandPoolHandle command_pool_{};
        cobalt::DescriptorAllocatorHandle descriptor_allocator_{};

        cobalt::PipelineLayoutHandle cubemap_sampling_pipeline_layout_{};
        cobalt::PipelineLayoutHandle sampling_pipeline_layout_{};
        cobalt::PipelineLayoutHandle processing_pipeline_layout_{};
        cobalt::PipelineHandle depth_prepass_pipeline_{};
        cobalt::PipelineHandle gbuffer_pass_pipeline_{};
        cobalt::PipelineHandle lighting_pass_pipeline_{};
        cobalt::PipelineHandle post_processing_pass_pipeline_{};

        cobalt::RendererHandle renderer_{};

        cobalt::ImageSamplerHandle texture_sampler_{};
        cobalt::ImageSamplerHandle shadow_map_sampler_{};
        cobalt::ImageCollectionHandle albedo_images_{};
        cobalt::ImageCollectionHandle material_images_{};
        cobalt::ImageCollectionHandle post_processing_images_{};
        cobalt::ImageCollectionHandle shadow_map_depth_images_{};
        cobalt::ImageHandle cube_skybox_image_{};
        cobalt::ImageHandle cube_diffuse_irradiance_image_{};
        cobalt::ModelHandle model_{};

        cobalt::BufferHandle lights_buffer_{};
        std::vector<cobalt::BufferHandle> camera_uniform_buffers_{};

        // .CREATION
        void create_descriptor_allocator( );
        void create_render_images( VkExtent2D extent );
        void create_shadow_map_images( uint32_t size );
        void create_uniform_buffers( );
        void create_pipelines( );

        void write_textures_descriptor_sets( );
        void write_cube_textures_descriptor_sets( cobalt::Image const& temp_image );
        void write_shadow_map_textures_descriptor_sets( );

        // .RENDERING
        void record_command_buffer(
            cobalt::CommandBuffer const&, cobalt::Swapchain&, uint32_t image_index, uint32_t frame_index );
        void render_to_cubemap( cobalt::Image& attachment, cobalt::shader::ShaderModule vert, cobalt::shader::ShaderModule frag );
        void render_skybox_map( );
        void render_irradiance_map( );
        void render_shadow_maps( );
        void update_camera_data( uint32_t current_image ) const;

        // .UTILITIES
        void viewport_changed( VkExtent2D extent );

        static void configure_relative_path( );

    };

}


#endif //MYAPPLICATION_H
