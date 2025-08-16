#include "MyApplication.h"

#include "Camera.h"
#include "debug_callback.h"
#include "Timer.h"
#include "UniformBufferObject.h"

#include <cobalt_vk/core.h>

#include <xos/filesystem.h>
#include <xos/info.h>

#include <iostream>
#include <__image/CubeMapImage.h>


using namespace cobalt;


// +---------------------------+
// | PUBLIC                    |
// +---------------------------+
MyApplication::MyApplication( )
{
    // 1. Create Window and Camera
    window_ = CVK.create_resource<Window>( WIDTH_, HEIGHT_, "Vulkan App" );

    camera_ptr_ = std::make_unique<Camera>( window_->handle( ), window_->extent( ), glm::radians( 45.f ),
                                            0.1f, 25.f, glm::vec3{ 0.f, 2.f, 0.f } );
    camera_ptr_->set_yaw( glm::radians( 90.f ) );
    camera_ptr_->set_pitch( glm::radians( -10.f ) );
    window_->on_framebuffer_resize.bind( camera_ptr_.get( ), &Camera::set_viewport );

    // 2. Register VK Instance
    constexpr VkApplicationInfo app_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "MyApplication",
        .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .pEngineName = "Cobalt",
        .engineVersion = VK_MAKE_VERSION( 1, 0, 0 ),
        .apiVersion = VK_API_VERSION_1_3
    };
    context_ = CVK.create_resource<VkContext>(
        ContextWizard{ { window_.get( ), app_info } }
        .with<DeviceFeatureFlags>(
            DeviceFeatureFlags::SWAPCHAIN_EXT | DeviceFeatureFlags::ANISOTROPIC_SAMPLING |
            DeviceFeatureFlags::DYNAMIC_RENDERING_EXT | DeviceFeatureFlags::SYNCHRONIZATION_2_EXT |
            DeviceFeatureFlags::SHADER_IMAGE_ARRAY_NON_UNIFORM_INDEXING )
        .with<ValidationLayers>( ValidationFlags::KHRONOS_VALIDATION, ::debug::debug_callback )
    );

    // 3. Set the proper root directory to find shader modules and textures.
    configure_relative_path( );

    // 4. Swapchain
    swapchain_ = CVK.create_resource<Swapchain>(
        SwapchainWizard{
            *context_,
            *window_,
            SwapchainCreateInfo{
                .image_count = 3,
                .present_mode = VK_PRESENT_MODE_MAILBOX_KHR,
                .surface_format = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
            }
        } );
    command_pool_ = CVK.create_resource<CommandPool>( *context_, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT );
    swapchain_->depth_image( ).transition_layout( { VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL }, *command_pool_ );

    // 5. Descriptors
    create_descriptor_allocator( );

    // 6. Renderer
    renderer_ = CVK.create_resource<Renderer>( RendererCreateInfo{
        .device = &context_->device( ),
        .cmd_pool = command_pool_.get( ),
        .swapchain = swapchain_.get( ),
        .max_frames_in_flight = MAX_FRAMES_IN_FLIGHT_
    } );
    renderer_->set_record_command_buffer_fn(
        std::bind( &MyApplication::record_command_buffer, this,
                   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4 ) );
    renderer_->set_update_uniform_buffer_fn(
        std::bind( &MyApplication::update_camera_data, this, std::placeholders::_1 ) );

    // 7. Sampler and Images
    texture_sampler_ = CVK.create_resource<ImageSampler>(
        context_->device( ),
        ImageSamplerCreateInfo{
            .filter = VK_FILTER_LINEAR,
        } );
    create_gbuffer_images( );
    create_post_processing_images( );

    // 8. Graphic pipelines
    create_pipelines( );

    // 9. Model
    model_ = CVK.create_resource<Model>( context_->device( ), *command_pool_, loader::AssimpModelLoader{ MODEL_PATH_ } );

    // 10. Uniform buffers
    for ( uint32_t i{}; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        camera_uniform_buffers_.emplace_back(
            CVK.create_resource<Buffer>( buffer::make_uniform_buffer( context_->device( ), sizeof( CameraData ) ) ) );
    }

    // 11. Update descriptor sets
    write_descriptor_sets( );
}


MyApplication::~MyApplication( ) noexcept
{
    // We wait for the device to finish all operations before cleaning up, since the render is asynchronous in the GPU.
    context_->device( ).wait_idle( );
    CVK.reset_instance( );
}


void MyApplication::run( )
{
    Timer timer{};

    // 1. Start the timer
    timer.start( );
    running_ = true;

    // 2. Render the cubemap image
    render_to_cubemap( cube_skybox_image_, SKYBOX_PATH_, "shaders/cubemap.vert.spv", "shaders/cubemap.frag.spv" );

    // 3. Start the render loop
    while ( running_ )
    {
        // 3.1 Poll glfw events
        glfwPollEvents( );

        // 3.2 Update timer and camera
        timer.update( );
        camera_ptr_->update( &timer );

        // 3.3 Skip rendering if the window is minimized
        if ( window_->is_minimized( ) )
        {
            continue;
        }

        // 3.4 Render
        if ( auto const render_result = renderer_->render( );
            render_result == VK_ERROR_OUT_OF_DATE_KHR || render_result == VK_SUBOPTIMAL_KHR )
        {
            window_->force_framebuffer_resize( );
        }

        // 3.5 Check if the window should close
        running_ = not window_->should_close( );
    }
}


// +---------------------------+
// | PRIVATE                   |
// +---------------------------+
void MyApplication::create_descriptor_allocator( )
{
    descriptor_allocator_ = CVK.create_resource<DescriptorAllocator>(
        descriptor::LayoutSpecs{ context_->device( ) }
        .define(
            "buffer_layout",
            {
                // Camera uniform buffer
                { VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },

                // Surface Maps Buffer
                { VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
            } )
        .define(
            "g_layout",
            {
                // Sampler
                { VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLER },

                // Textures
                { VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, TEXTURES_COUNT_ },

                // Swapchain Depth Image
                { VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE },

                // Albedo Image Buffer
                { VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE },

                // Material Image Buffer
                { VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE },

                // HDR Post Processing Image
                { VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE },
            } )
        .define(
            "cubemap_layout",
            {
                // Cubemap HDR Image
                { VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE },

                // Cubemap Cube Image
                { VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE },
            } )
        .alloc( "buffer", "buffer_layout", MAX_FRAMES_IN_FLIGHT_ )
        .alloc( "g", "g_layout", MAX_FRAMES_IN_FLIGHT_ )
        .alloc( "cubemap", "cubemap_layout", 1u ) );
}


void MyApplication::create_gbuffer_images( )
{
    albedo_images_ = CVK.create_resource<ImageCollection>(
        context_->device( ), ImageCreateInfo{
            .extent = swapchain_->extent( ),
            .format = VK_FORMAT_R8G8B8A8_SRGB,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT
        }, MAX_FRAMES_IN_FLIGHT_ );

    material_images_ = CVK.create_resource<ImageCollection>(
        context_->device( ), ImageCreateInfo{
            .extent = swapchain_->extent( ),
            .format = VK_FORMAT_R16G16B16A16_UNORM,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT
        }, MAX_FRAMES_IN_FLIGHT_ );
}


void MyApplication::create_post_processing_images( )
{
    hdr_images_ = CVK.create_resource<ImageCollection>(
        context_->device( ), ImageCreateInfo{
            .extent = swapchain_->extent( ),
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT
        }, MAX_FRAMES_IN_FLIGHT_ );
}


void MyApplication::create_pipelines( )
{
    // Layouts
    {
        DescriptorSet const* const buffer_set  = &descriptor_allocator_->set_at( "buffer" );
        DescriptorSet const* const g_set       = &descriptor_allocator_->set_at( "g" );
        DescriptorSet const* const cubemap_set = &descriptor_allocator_->set_at( "cubemap" );

        sampling_pipeline_layout_ = CVK.create_resource<PipelineLayout>(
            context_->device( ), std::array{ buffer_set, g_set },
            std::array{
                // Surface ID
                VkPushConstantRange{
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .offset = 0u,
                    .size = sizeof( uint32_t )
                }
            } );

        processing_pipeline_layout_ = CVK.create_resource<PipelineLayout>(
            context_->device( ), std::array{ buffer_set, g_set, cubemap_set },
            std::array{
                // Camera position
                VkPushConstantRange{
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .offset = 0u,
                    .size = sizeof( glm::vec3 )
                }
            } );

        cubemap_pipeline_layout_ = CVK.create_resource<PipelineLayout>(
            context_->device( ), std::array{ buffer_set, g_set, cubemap_set },
            std::array{
                // Proj / View
                VkPushConstantRange{
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .offset = 0u,
                    .size = sizeof( glm::mat4 ) * 2u
                },
            } );
    }

    // Depth pre-pass pipeline
    {
        constexpr VkSpecializationMapEntry specialization_entry{
            .constantID = 0u,
            .offset = 0u,
            .size = sizeof( TEXTURES_COUNT_ ),
        };

        VkSpecializationInfo const specialization_info{
            .mapEntryCount = 1u,
            .pMapEntries = &specialization_entry,
            .dataSize = sizeof( TEXTURES_COUNT_ ),
            .pData = &TEXTURES_COUNT_
        };

        depth_prepass_pipeline_ = CVK.create_resource<Pipeline>(
            builder::GraphicsPipelineBuilder{}
            .add_shader_module( { context_->device( ), "shaders/transform.vert.spv", VK_SHADER_STAGE_VERTEX_BIT } )
            .add_shader_module( { context_->device( ), "shaders/depth.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT },
                                &specialization_info )
            .set_dynamic_state( std::array{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } )
            .set_binding_description( Vertex::get_binding_description( ), Vertex::get_attribute_descriptions( ) )
            .set_depth_stencil_mode( VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS )
            .set_depth_image_description( swapchain_->depth_image( ).format( ) )
            .build( context_->device( ), *sampling_pipeline_layout_, VK_PIPELINE_BIND_POINT_GRAPHICS ) );
    }

    // G-Buffer generation pipeline
    {
        constexpr VkSpecializationMapEntry specialization_entry{
            .constantID = 0u,
            .offset = 0u,
            .size = sizeof( TEXTURES_COUNT_ ),
        };

        VkSpecializationInfo const specialization_info{
            .mapEntryCount = 1u,
            .pMapEntries = &specialization_entry,
            .dataSize = sizeof( TEXTURES_COUNT_ ),
            .pData = &TEXTURES_COUNT_
        };

        gbuffer_pass_pipeline_ = CVK.create_resource<Pipeline>(
            builder::GraphicsPipelineBuilder{}
            .add_shader_module( { context_->device( ), "shaders/transform.vert.spv", VK_SHADER_STAGE_VERTEX_BIT } )
            .add_shader_module( { context_->device( ), "shaders/gbuffer_gen.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT },
                                &specialization_info )
            .set_dynamic_state( std::array{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } )
            .set_binding_description( Vertex::get_binding_description( ), Vertex::get_attribute_descriptions( ) )
            .set_depth_stencil_mode( VK_TRUE, VK_FALSE, VK_COMPARE_OP_EQUAL )
            .set_depth_image_description( swapchain_->depth_image( ).format( ) )
            .add_color_attachment_description(
                VkPipelineColorBlendAttachmentState{
                    .blendEnable = VK_FALSE,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                      VK_COLOR_COMPONENT_A_BIT,
                }, albedo_images_->image_format( ) )
            .add_color_attachment_description(
                VkPipelineColorBlendAttachmentState{
                    .blendEnable = VK_FALSE,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                      VK_COLOR_COMPONENT_A_BIT,
                }, material_images_->image_format( ) )
            .build( context_->device( ), *sampling_pipeline_layout_, VK_PIPELINE_BIND_POINT_GRAPHICS ) );
    }

    // Lighting pass pipeline
    {
        lighting_pass_pipeline_ = CVK.create_resource<Pipeline>(
            builder::GraphicsPipelineBuilder{}
            .add_shader_module( { context_->device( ), "shaders/quad.vert.spv", VK_SHADER_STAGE_VERTEX_BIT } )
            .add_shader_module( { context_->device( ), "shaders/lighting.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } )
            .set_dynamic_state( std::array{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } )
            .set_depth_stencil_mode( VK_FALSE, VK_FALSE )
            .set_cull_mode( VK_CULL_MODE_NONE )
            .add_color_attachment_description(
                VkPipelineColorBlendAttachmentState{
                    .blendEnable = VK_FALSE,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                      VK_COLOR_COMPONENT_A_BIT,
                }, hdr_images_->image_format( ) )
            .build( context_->device( ), *processing_pipeline_layout_, VK_PIPELINE_BIND_POINT_GRAPHICS ) );
    }

    // Post-processing pass pipeline
    {
        post_processing_pass_pipeline_ = CVK.create_resource<Pipeline>(
            builder::GraphicsPipelineBuilder{}
            .add_shader_module( { context_->device( ), "shaders/quad.vert.spv", VK_SHADER_STAGE_VERTEX_BIT } )
            .add_shader_module( { context_->device( ), "shaders/tone_mapping.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT } )
            .set_dynamic_state( std::array{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } )
            .set_depth_stencil_mode( VK_FALSE, VK_FALSE )
            .set_cull_mode( VK_CULL_MODE_NONE )
            .add_color_attachment_description(
                VkPipelineColorBlendAttachmentState{
                    .blendEnable = VK_FALSE,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                      VK_COLOR_COMPONENT_A_BIT,
                }, swapchain_->image_format( ) )
            .build( context_->device( ), *processing_pipeline_layout_, VK_PIPELINE_BIND_POINT_GRAPHICS ) );
    }
}


void MyApplication::write_descriptor_sets( )
{
    // Buffer descriptors
    {
        std::array write_ops{
            WriteDescription{
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                [this]( uint32_t const frame_index ) -> VkDescriptorBufferInfo
                    {
                        return {
                            .buffer = camera_uniform_buffers_[frame_index]->handle( ),
                            .offset = 0u,
                            .range = sizeof( CameraData ),
                        };
                    }
            },
            WriteDescription{
                VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                [this]( uint32_t ) -> VkDescriptorBufferInfo
                    {
                        return {
                            .buffer = model_->surface_buffer( ).handle( ),
                            .offset = 0u,
                            .range = model_->surface_buffer( ).memory_size( )
                        };
                    }
            },
        };
        descriptor_allocator_->set_at( "buffer" ).update( write_ops );
    }

    // Sampled descriptors
    {
        std::array write_ops{
            WriteDescription{
                VK_DESCRIPTOR_TYPE_SAMPLER,
                [this]( uint32_t ) -> VkDescriptorImageInfo
                    {
                        return {
                            .sampler = texture_sampler_->handle( )
                        };
                    },
            },
            WriteDescription{
                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                [this]( uint32_t ) -> std::vector<VkDescriptorImageInfo>
                    {
                        auto const texture_images = model_->textures( );
                        std::vector<VkDescriptorImageInfo> infos;
                        infos.reserve( texture_images.size( ) );
                        for ( auto const& tex : texture_images )
                        {
                            infos.push_back( {
                                .imageView = tex.image( ).view( ).handle( ),
                                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                            } );
                        }
                        return infos;
                    }
            },
            WriteDescription{
                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                [this]( uint32_t ) -> VkDescriptorImageInfo
                    {
                        return {
                            .imageView = swapchain_->depth_image( ).view( ).handle( ),
                            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                        };
                    }
            },
            WriteDescription{
                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                [this]( uint32_t const frame_index ) -> VkDescriptorImageInfo
                    {
                        return {
                            .imageView = albedo_images_->image_at( frame_index ).view( ).handle( ),
                            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                        };
                    }
            },
            WriteDescription{
                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                [this]( uint32_t const frame_index ) -> VkDescriptorImageInfo
                    {
                        return {
                            .imageView = material_images_->image_at( frame_index ).view( ).handle( ),
                            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                        };
                    }
            },
            WriteDescription{
                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                [this]( uint32_t const frame_index ) -> VkDescriptorImageInfo
                    {
                        return {
                            .imageView = hdr_images_->image_at( frame_index ).view( ).handle( ),
                            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                        };
                    }
            },
        };
        descriptor_allocator_->set_at( "g" ).update( write_ops );
    }
}


void MyApplication::record_command_buffer( CommandBuffer const& buffer, Swapchain& swapchain,
                                           uint32_t const image_index, uint32_t const frame_index )
{
    buffer.reset( );

    CommandOperator command_op = buffer.command_operator( 0 );

    command_op.store_render_area( VkRect2D{ .offset = { 0u, 0u }, .extent = swapchain.extent( ) } );
    command_op.store_viewport( VkViewport{
        .x = 0.f, .y = 0.f,
        .width = static_cast<float>( swapchain.extent( ).width ),
        .height = static_cast<float>( swapchain.extent( ).height ),
        .minDepth = 0.f, .maxDepth = 1.f
    } );

    Image& albedo_image   = albedo_images_->image_at( frame_index );
    Image& material_image = material_images_->image_at( frame_index );
    Image& hdr_image      = hdr_images_->image_at( frame_index );
    Image& swap_image     = swapchain.image_at( image_index );

    // 1. Depth Pre-Pass: render geometry to depth only, no color attachment
    {
        // DEPTH STENCIL READONLY OPTIMAL -> DEPTH STENCIL ATTACHMENT OPTIMAL
        swapchain_->depth_image( ).transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT )
            .from_access( VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT )
            .to_access( VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ), command_op );

        VkRenderingAttachmentInfo const depth_attachment =
                swapchain_->depth_image( ).view( ).make_depth_attachment( VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                                          VK_ATTACHMENT_STORE_OP_STORE );

        command_op.begin_rendering( {}, &depth_attachment );

        command_op.set_viewport( );
        command_op.set_scissor( );

        command_op.bind_pipeline( *depth_prepass_pipeline_, frame_index );
        command_op.bind_vertex_buffers( model_->vertex_buffer( ), 0 );
        command_op.bind_index_buffer( model_->index_buffer( ), 0 );

        for ( auto const& [index_count, index_offset, vertex_offset, material_index] : model_->meshes( ) )
        {
            command_op.push_constants(
                *depth_prepass_pipeline_, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof( uint32_t ), &material_index );
            command_op.draw_indexed( index_count, 1, index_offset, vertex_offset );
        }

        command_op.end_rendering( );

        // DEPTH STENCIL ATTACHMENT OPTIMAL -> DEPTH STENCIL READONLY OPTIMAL
        swapchain_->depth_image( ).transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT )
            .from_access( VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT )
            .to_access( VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT ), command_op );
    }

    // 2. G-Buffer generation pass: color on g-buffer images
    {
        // SHADER READONLY OPTIMAL -> COLOR ATTACHMENT OPTIMAL
        albedo_image.transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT )
            .from_access( VK_ACCESS_2_SHADER_SAMPLED_READ_BIT )
            .to_access( VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT ),
            command_op );
        material_image.transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT )
            .from_access( VK_ACCESS_2_SHADER_SAMPLED_READ_BIT )
            .to_access( VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT ),
            command_op );

        std::array const color_attachments{
            albedo_image.view( ).make_color_attachment( VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE ),
            material_image.view( ).make_color_attachment( VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE )
        };
        VkRenderingAttachmentInfo const depth_attachment =
                swapchain_->depth_image( ).view( ).make_depth_attachment( VK_ATTACHMENT_LOAD_OP_LOAD,
                                                                          VK_ATTACHMENT_STORE_OP_DONT_CARE );

        command_op.begin_rendering( color_attachments, &depth_attachment );

        command_op.set_viewport( );
        command_op.set_scissor( );

        command_op.bind_pipeline( *gbuffer_pass_pipeline_, frame_index );
        command_op.bind_vertex_buffers( model_->vertex_buffer( ), 0 );
        command_op.bind_index_buffer( model_->index_buffer( ), 0 );

        for ( auto const& [index_count, index_offset, vertex_offset, material_index] : model_->meshes( ) )
        {
            command_op.push_constants(
                *gbuffer_pass_pipeline_, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof( uint32_t ), &material_index );
            command_op.draw_indexed( index_count, 1, index_offset, vertex_offset );
        }

        command_op.end_rendering( );

        // COLOR ATTACHMENT OPTIMAL -> SHADER READONLY OPTIMAL
        albedo_image.transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT )
            .from_access( VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT )
            .to_access( VK_ACCESS_2_SHADER_SAMPLED_READ_BIT ),
            command_op );
        material_image.transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT )
            .from_access( VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT )
            .to_access( VK_ACCESS_2_SHADER_SAMPLED_READ_BIT ),
            command_op );
    }

    // 3. Lighting pass: color + depth read-only
    {
        // SHADER READONLY OPTIMAL -> COLOR ATTACHMENT OPTIMAL
        hdr_image.transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_TRANSFER_BIT )
            .from_access( VK_ACCESS_2_NONE )
            .to_access( VK_ACCESS_2_TRANSFER_WRITE_BIT ), command_op );

        VkRenderingAttachmentInfo const color_attachment =
                hdr_image.view( ).make_color_attachment( VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE );

        command_op.begin_rendering( std::array{ color_attachment }, nullptr );

        command_op.set_viewport( );
        command_op.set_scissor( );

        command_op.bind_pipeline( *lighting_pass_pipeline_, frame_index );

        command_op.push_constants(
            *lighting_pass_pipeline_, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof( glm::vec3 ), &camera_ptr_->eye( ) );
        command_op.draw( 4, 1 );

        command_op.end_rendering( );

        // COLOR ATTACHMENT OPTIMAL -> SHADER READONLY OPTIMAL
        hdr_image.transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT )
            .from_access( VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT )
            .to_access( VK_ACCESS_2_NONE ), command_op );
    }

    // 4. Post-processing pass: tone mapping
    {
        // PRESENT -> COLOR ATTACHMENT OPTIMAL
        swap_image.transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
            .from_stage( VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_TRANSFER_BIT )
            .from_access( VK_ACCESS_2_NONE )
            .to_access( VK_ACCESS_2_TRANSFER_WRITE_BIT ), command_op );

        VkRenderingAttachmentInfo const color_attachment =
                swap_image.view( ).make_color_attachment( VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE );

        command_op.begin_rendering( std::array{ color_attachment }, nullptr );

        command_op.set_viewport( );
        command_op.set_scissor( );

        command_op.bind_pipeline( *post_processing_pass_pipeline_, frame_index );

        command_op.draw( 4, 1 );

        command_op.end_rendering( );

        // COLOR ATTACHMENT OPTIMAL -> PRESENT
        swap_image.transition_layout(
            ImageLayoutTransition{ VK_IMAGE_LAYOUT_PRESENT_SRC_KHR }
            .from_stage( VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT )
            .to_stage( VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT )
            .from_access( VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT )
            .to_access( VK_ACCESS_2_NONE ), command_op );
    }
}


void MyApplication::render_to_cubemap( ImageHandle& render_target, std::filesystem::path const& path_to_image,
                                       std::filesystem::path const& path_to_shader_vert,
                                       std::filesystem::path const& path_to_shader_frag )
{
    // Load HDR cubemap image
    CubeMapImage cubemap_image{
        context_->device( ), *command_pool_,
        CubeMapImageCreateInfo{
            .path_to_img = path_to_image
        }
    };

    // Update descriptor sets
    {
        std::array write_ops{
            WriteDescription{
                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                [&cubemap_image]( uint32_t ) -> VkDescriptorImageInfo
                    {
                        return {
                            .imageView = cubemap_image.hdr_image( ).view( ).handle( ),
                            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                        };
                    }
            },
            WriteDescription{
                VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                [&cubemap_image]( uint32_t ) -> VkDescriptorImageInfo
                    {
                        return {
                            .imageView = cubemap_image.cube_image( ).view( ).handle( ),
                            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                        };
                    }
            },
        };
        descriptor_allocator_->set_at( "cubemap" ).update( write_ops );
    }

    // Create Cubemap Pipeline
    Pipeline const cubemap_pipeline{
        builder::GraphicsPipelineBuilder{}
        .add_shader_module( { context_->device( ), path_to_shader_vert, VK_SHADER_STAGE_VERTEX_BIT } )
        .add_shader_module( { context_->device( ), path_to_shader_frag, VK_SHADER_STAGE_FRAGMENT_BIT } )
        .set_dynamic_state( std::array{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } )
        .set_depth_stencil_mode( VK_FALSE, VK_FALSE )
        .set_cull_mode( VK_CULL_MODE_NONE )
        .add_color_attachment_description(
            VkPipelineColorBlendAttachmentState{
                .blendEnable = VK_FALSE,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                  VK_COLOR_COMPONENT_A_BIT,
            }, cubemap_image.cube_image( ).format( ) )
        .build( context_->device( ), *cubemap_pipeline_layout_, VK_PIPELINE_BIND_POINT_GRAPHICS )
    };

    // Render pass
    {
        glm::mat4 const proj = camera_ptr_->projection( );

        glm::vec3 const eye = camera_ptr_->eye( );
        glm::mat4 const views[6]{
            lookAt( eye, eye + glm::vec3{ 1.f, 0.f, 0.f }, glm::vec3{ 0.f, -1.f, 0.f } ),  // +X
            lookAt( eye, eye + glm::vec3{ -1.f, 0.f, 0.f }, glm::vec3{ 0.f, -1.f, 0.f } ), // -X
            lookAt( eye, eye + glm::vec3{ 0.f, -1.f, 0.f }, glm::vec3{ 0.f, 0.f, -1.f } ), // -Y
            lookAt( eye, eye + glm::vec3{ 0.f, 1.f, 0.f }, glm::vec3{ 0.f, 0.f, 1.f } ),   // +Y
            lookAt( eye, eye + glm::vec3{ 0.f, 0.f, 1.f }, glm::vec3{ 0.f, -1.f, 0.f } ),  // +Z
            lookAt( eye, eye + glm::vec3{ 0.f, 0.f, -1.f }, glm::vec3{ 0.f, -1.f, 0.f } ), // -Z
        };

        auto const& cmd_buffer = command_pool_->acquire( VK_COMMAND_BUFFER_LEVEL_PRIMARY );
        cmd_buffer.reset( );

        std::array<ImageView, 6> const image_views = cubemap_image.generate_cubic_views( context_->device( ) );

        // Start recording command buffer
        {
            CommandOperator command_op = cmd_buffer.command_operator( 0 );

            command_op.store_render_area( VkRect2D{ .offset = { 0u, 0u }, .extent = cubemap_image.cube_image( ).extent( ) } );
            command_op.store_viewport( VkViewport{
                .x = 0.f, .y = 0.f,
                .width = static_cast<float>( cubemap_image.cube_image( ).extent( ).width ),
                .height = static_cast<float>( cubemap_image.cube_image( ).extent( ).height ),
                .minDepth = 0.f, .maxDepth = 1.f
            } );


            // UNDEFINED -> COLOR ATTACHMENT OPTIMAL
            cubemap_image.cube_image( ).transition_layout(
                ImageLayoutTransition{ VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
                .from_stage( VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT )
                .to_stage( VK_PIPELINE_STAGE_2_TRANSFER_BIT )
                .from_access( VK_ACCESS_2_NONE )
                .to_access( VK_ACCESS_2_TRANSFER_WRITE_BIT ), command_op );
            for ( uint32_t view_index{}; view_index < 6; view_index++ )
            {
                VkRenderingAttachmentInfo const color_attachment =
                        image_views[view_index].
                        make_color_attachment( VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE );

                command_op.begin_rendering( std::array{ color_attachment }, nullptr );

                command_op.set_viewport( );
                command_op.set_scissor( );

                command_op.bind_pipeline( cubemap_pipeline, 0u );

                command_op.push_constants( cubemap_pipeline, VK_SHADER_STAGE_VERTEX_BIT, 0u, sizeof( glm::mat4 ), &proj );
                command_op.push_constants(
                    cubemap_pipeline, VK_SHADER_STAGE_VERTEX_BIT, sizeof( glm::mat4 ), sizeof( glm::mat4 ), &views[view_index] );
                command_op.draw( 36, 1 );

                command_op.end_rendering( );
            }
            // COLOR ATTACHMENT OPTIMAL -> SHADER READONLY OPTIMAL
            cubemap_image.cube_image( ).transition_layout(
                ImageLayoutTransition{ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL }
                .from_stage( VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT )
                .to_stage( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT )
                .from_access( VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT )
                .to_access( VK_ACCESS_2_SHADER_SAMPLED_READ_BIT ), command_op );
        }

        sync::Fence const fence{ context_->device( ) };
        context_->device( ).graphics_queue( ).submit(
            sync::SubmitInfo{ context_->device( ).device_index( ) }.execute( cmd_buffer ), &fence );

        fence.wait( );
        cmd_buffer.unlock( );
    }

    // Transfer image ownership
    render_target = CVK.create_resource<Image>( cubemap_image.steal_cubemap_image( ) );
}


void MyApplication::update_camera_data( uint32_t const current_image ) const
{
    CameraData const ubo{
        .model = glm::mat4( 1.0f ), //rotate( glm::mat4( 1.0f ), glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
        .view = camera_ptr_->camera_to_world( ),
        .proj = camera_ptr_->projection( )
    };
    camera_uniform_buffers_[current_image]->write( &ubo, sizeof( CameraData ) );
}


void MyApplication::configure_relative_path( )
{
    xos::info::log_info( std::clog );
    xos::filesystem::configure_relative_path( );
}
