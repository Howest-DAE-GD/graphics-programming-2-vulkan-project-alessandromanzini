#include <TriangleApplication.h>

// +---------------------------+
// | STANDARD HEADERS          |
// +---------------------------+
#include <chrono>
#include <functional>
#include <iostream>
#include <set>
#include <stdexcept>

// +---------------------------+
// | PROJECT HEADERS           |
// +---------------------------+
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ShaderModules.h>
#include <VulkanDeviceQueries.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <assets/Buffer.h>

#include <xos/filesystem.h>
#include <xos/info.h>

#include <filesystem>

#include <CobaltVK.h>
#include <__builder/ModelLoader.h>

#include <SingleTimeCommand.h>
#include <UniformBufferObject.h>
#include <__image/ImageView.h>
#include <__swapchain/Swapchain.h>
#include <__validation/result.h>

#include "../../cobalt/include/private/__query/queue_family.h"
#include "../../cobalt/include/private/__query/swapchain_support.h"


using namespace cobalt;


// +---------------------------+
// | PUBLIC                    |
// +---------------------------+
TriangleApplication::TriangleApplication( )
{
    // 1. Create Window
    window_ = CVK.create_resource<Window>( WIDTH_, HEIGHT_, "Vulkan App" );

    // 2. Register VK Instance
    context_ = CVK.create_resource<VkContext>( ContextCreateInfo{
        .window = window_.get( ),
        .app_info = VkApplicationInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Hello Viking",
            .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
            .pEngineName = "Cobalt",
            .engineVersion = VK_MAKE_VERSION( 1, 0, 0 ),
            .apiVersion = VK_API_VERSION_1_3
        },
        .validation_layers = ValidationLayers{ validation_layers_, debug_callback }
    } );
    context_->create_device( DeviceFeatureFlags::SWAPCHAIN_EXTENSION | DeviceFeatureFlags::ANISOTROPIC_SAMPLING
                             | DeviceFeatureFlags::DYNAMIC_RENDERING_EXTENSION );

    // 3. Set the proper root directory to find shader modules and textures.
    configure_relative_path( );
}


void TriangleApplication::run( )
{
    init_vk( );

    bool keep_running{ true };
    while ( keep_running )
    {
        keep_running = not window_->should_close( );
        main_loop( );
    }
    // All the operations in drawFrame are asynchronous. When we exit the loop, drawing and presentation operations may
    // still be going on. We solve it by waiting for the logical device to finish operations before exiting.
    context_->device( ).wait_idle( );

    cleanup( );
}


// +---------------------------+
// | PRIVATE                   |
// +---------------------------+


void TriangleApplication::vk_recreate_swap_chain( )
{
    // todo: remove
    // In case the window gets minimized, we wait until it gets restored.
    auto [width, height] = window_->extent( );
    // perhaps use thread sleep instead of while
    while ( width == 0 || height == 0 )
    {
        auto [tempWidth, tempHeight] = window_->extent( );
        width                        = tempWidth;
        height                       = tempHeight;
        glfwWaitEvents( );
    }

    // It is possible to create a new swap chain while drawing commands on an image from the old swap chain are still
    // in-flight. You need to pass the previous swap chain to the oldSwapChain field in the VkSwapchainCreateInfoKHR
    // struct and destroy the old swap chain as soon as you've finished using it.
    vkDeviceWaitIdle( context_->device( ).logical( ) );

    // vk_cleanup_swap_chain( );
    //
    // vk_create_swap_chain( );
    // vk_create_image_views( );
    vk_create_depth_resources( );
    vk_create_frame_buffers( );
}


void TriangleApplication::vk_create_render_pass( )
{
    VkAttachmentDescription color_attachment{};

    // The format of the color attachment should match the format of the swap chain images, and we're not doing anything
    // with multisampling yet, so we'll stick to 1 sample.
    color_attachment.format  = swapchain_ptr_->image_format( );
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

    // The loadOp and storeOp determine what to do with the data in the attachment before rendering and after rendering.
    // The loadOp and storeOp apply to color and depth data, and stencilLoadOp / stencilStoreOp apply to stencil data
    color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // A single render pass can consist of multiple sub-passes. Sub-passes are subsequent rendering operations that
    // depend on the contents of frame buffers in previous passes
    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth buffer attachment
    VkAttachmentDescription depth_attachment{};
    depth_attachment.format         = query::find_depth_format( context_->device( ).physical( ) );
    depth_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    // Remember that the subpasses in a render pass automatically take care of image layout transitions. These
    // transitions are controlled by subpass dependencies, which specify memory and execution dependencies between
    // subpasses.Subpass dependencies are specified in VkSubpassDependency structs.
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Now we create a render pass object by filling in the VkRenderPassCreateInfo structure with out attachments and
    // sub-passes.
    std::array const attachments{ color_attachment, depth_attachment };
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>( attachments.size( ) );
    render_pass_info.pAttachments    = attachments.data( );
    render_pass_info.subpassCount    = 1;
    render_pass_info.pSubpasses      = &subpass;

    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies   = &dependency;

    // Make dependency referer to the attachments
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    validation::throw_on_bad_result(
        vkCreateRenderPass( context_->device( ).logical( ), &render_pass_info, nullptr, &render_pass_ ),
        "Failed to create render pass!" );
}


void TriangleApplication::vk_create_descriptor_set_layout( )
{
    VkDescriptorSetLayoutBinding ubo_layout_binding{};

    // The first two fields specify the binding used in the shader and the type of descriptor, which is a uniform buffer
    // object. It is possible for the shader variable to represent an array of uniform buffer objects, and
    // descriptorCount specifies the number of values in the array.
    ubo_layout_binding.binding         = 0;
    ubo_layout_binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;

    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding sampler_layout_binding{};
    sampler_layout_binding.binding            = 1;
    sampler_layout_binding.descriptorCount    = 1;
    sampler_layout_binding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = nullptr;
    sampler_layout_binding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array const bindings = { ubo_layout_binding, sampler_layout_binding };

    // We need to specify the descriptor set layout during pipeline creation to tell Vulkan which descriptors the
    // shaders will be using.
    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>( bindings.size( ) );
    layout_info.pBindings    = bindings.data( );

    validation::throw_on_bad_result(
        vkCreateDescriptorSetLayout( context_->device( ).logical( ), &layout_info, nullptr, &descriptor_set_layout_ ),
        "Failed to create descriptor set layout!" );
}


void TriangleApplication::vk_create_graphics_pipeline( )
{
    auto const vert_shader_code = shader::read_file( "shaders/shader.vert.spv" );
    auto const frag_shader_code = shader::read_file( "shaders/shader.frag.spv" );

    // Shader modules are just a thin wrapper around the shader bytecode that we've previously loaded from a file and
    // the functions defined in it.
    VkShaderModule vert_shader_module = shader::create_shader_module( context_->device( ).logical( ), vert_shader_code );
    VkShaderModule frag_shader_module = shader::create_shader_module( context_->device( ).logical( ), frag_shader_code );

    // We assign the shaders to specific pipelines stages through the shaderStages member of the
    // VkPipelineShaderStageCreateInfo
    VkPipelineShaderStageCreateInfo shader_stages[2] = {};

    // 0. Vertex Shader
    shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;

    shader_stages[0].module = vert_shader_module;
    shader_stages[0].pName  = "main";

    // 1. Fragment Shader
    shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

    shader_stages[1].module = frag_shader_module;
    shader_stages[1].pName  = "main";

    // The VkPipelineVertexInputStateCreateInfo structure describes the format of the vertex data passed to the vertex
    // shader. It describes this in roughly two ways:
    // 1. Bindings: spacing between data and whether the data is per-vertex or per-instance.
    // 2. Attribute descriptions: type of the attributes passed to the vertex shader, which binding to load them from
    // and at which offset.
    auto binding_description    = Vertex::get_binding_description( );
    auto attribute_descriptions = Vertex::get_attribute_descriptions( );

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.pVertexBindingDescriptions      = &binding_description;
    vertex_input_info.vertexBindingDescriptionCount   = 1;
    vertex_input_info.pVertexAttributeDescriptions    = attribute_descriptions.data( );
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>( attribute_descriptions.size( ) );

    // The VkPipelineInputAssemblyStateCreateInfo struct describes two things: what kind of geometry will be drawn from
    // the vertices and if primitive restart should be enabled.
    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    // At last, we specify their count at pipeline creation time.
    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount  = 1;

    // If we were to define the scissors and viewports statically, we would have to create a new viewportState every
    // time we need a change
    // viewportState.pViewports = &viewport;
    // viewportState.pScissors  = &scissor;

    // The rasterizer takes the geometry that is shaped by the vertices from the vertex shader and turns it into
    // fragments to be colored by the fragment shader. It also performs depth testing, face culling and the scissor
    // test.
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;

    // Culling
    rasterizer.cullMode  = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    // The VkPipelineMultisampleStateCreateInfo struct configures multisampling, which is one of the ways to perform
    // antialiasing.
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;     // Optional
    multisampling.pSampleMask           = nullptr;  // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable      = VK_FALSE; // Optional

    // After a fragment shader has returned a color, it needs to be combined with the color that is already in the
    // framebuffer. This transformation is known as color blending and there are two ways to do it:
    // 1. Mix the old and new value to produce a final color.
    // 2. Combine the old and new value using a bitwise operation.
    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
                                            | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable         = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;      // Optional
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;      // Optional

    // The second structure references the array of structures for all the frame buffers and allows you to set blend
    // constants that you can use as blend factors.
    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable     = VK_FALSE;
    color_blending.logicOp           = VK_LOGIC_OP_COPY; // Optional
    color_blending.attachmentCount   = 1;
    color_blending.pAttachments      = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f; // Optional
    color_blending.blendConstants[1] = 0.0f; // Optional
    color_blending.blendConstants[2] = 0.0f; // Optional
    color_blending.blendConstants[3] = 0.0f; // Optional

    // Dynamic state represents the states that can be changed without recreating the pipeline.
    // This setup will cause the configuration of these values to be ignored, and you will be able (and required) to
    // specify the data at drawing time. This results in a more flexible setup and is very common for things like
    // viewport and scissor state, which would result in a more complex setup when being baked into the pipeline state.
    std::vector dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>( dynamic_states.size( ) );
    dynamic_state.pDynamicStates    = dynamic_states.data( );

    // You can use uniform values in shaders, which are globals similar to dynamic state variables that can be changed
    // at drawing time to alter the behavior of your shaders without having to recreate them. They are commonly used to
    // pass the transformation matrix to the vertex shader, or to create texture samplers in the fragment shader.
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts    = &descriptor_set_layout_;

    validation::throw_on_bad_result(
        vkCreatePipelineLayout( context_->device( ).logical( ), &pipeline_layout_info, nullptr, &pipeline_layout_ ),
        "Failed to create pipeline layout!" );

    // Depth stencil creation
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable  = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;

    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;

    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.minDepthBounds        = 0.0f; // Optional
    depth_stencil.maxDepthBounds        = 1.0f; // Optional

    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.front             = {}; // Optional
    depth_stencil.back              = {}; // Optional

    // We can now combine all the structures and objects to create the graphics pipeline.
    // 1. Shader stages: the shader modules that define the functionality of the programmable stages of the
    // graphics pipeline.
    // 2. Fixed-function state: all the structures that define the fixed-function stages of the pipeline, like input
    // assembly, rasterizer, viewport and color blending.
    // 3. Pipeline layout: the uniform and push values referenced by the shader that can be updated at draw time.
    // 4. Render pass: the attachments referenced by the pipeline stages and their usage.

    // 1. We start by referencing the array of VkPipelineShaderStageCreateInfo structs.
    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType      = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages    = shader_stages;

    // 2. Then we reference all the structures describing the fixed-function stage.
    pipeline_info.pVertexInputState   = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState      = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState   = &multisampling;
    pipeline_info.pDepthStencilState  = nullptr; // Optional
    pipeline_info.pColorBlendState    = &color_blending;
    pipeline_info.pDynamicState       = &dynamic_state;

    // 3. After that comes the pipeline layout, which is a Vulkan handle rather than a struct pointer.
    pipeline_info.layout = pipeline_layout_;

    // 4. And finally we have the reference to the render pass and the index of the sub pass where this graphics
    // pipeline will be used.
    pipeline_info.renderPass = render_pass_;
    pipeline_info.subpass    = 0;

    // There are actually two optional more parameters: basePipelineHandle and basePipelineIndex. Vulkan allows you to
    // create a new graphics pipeline by deriving from an existing pipeline. The idea of pipeline derivatives is that
    // it is less expensive to set up pipelines when they have much functionality in common with an existing pipeline
    // and switching between pipelines from the same parent can also be done quicker.
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipeline_info.basePipelineIndex  = -1;             // Optional

    pipeline_info.pDepthStencilState = &depth_stencil;

    // The vkCreateGraphicsPipelines is designed to take multiple VkGraphicsPipelineCreateInfo objects and create
    // multiple VkPipeline objects in a single call.
    // The second parameter references an optional VkPipelineCache object. A pipeline cache can be used to store and
    // reuse data relevant to pipeline creation across multiple calls to vkCreateGraphicsPipelines.
    validation::throw_on_bad_result(
        vkCreateGraphicsPipelines( context_->device( ).logical( ), VK_NULL_HANDLE, 1, &pipeline_info, nullptr,
                                   &graphics_pipeline_ ),
        "Failed to create graphics pipeline!" );

    // The compilation and linking of the SPIR-V bytecode to machine code for execution
    // by the GPU doesn't happen until the graphics pipeline is created. That means that we're allowed to destroy the
    // shader modules again as soon as pipeline creation is finished.
    vkDestroyShaderModule( context_->device( ).logical( ), frag_shader_module, nullptr );
    vkDestroyShaderModule( context_->device( ).logical( ), vert_shader_module, nullptr );
}


void TriangleApplication::load_model( )
{
    model_ = CVK.create_resource<Model>( context_->device( ) );
    builder::ModelLoader const loader{ MODEL_PATH_ };
    loader.load( *model_ );

    model_->create_vertex_buffer( command_pool_, context_->device( ).graphics_queue( ) );
}


void TriangleApplication::vk_create_texture_image( )
{
    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load( "resources/viking_room.png", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha );
    VkDeviceSize const image_size = tex_width * tex_height * 4;

    if ( not pixels )
    {
        throw std::runtime_error( "Failed to load texture image!" );
    }

    Buffer::vk_create_buffer( context_->device( ).logical( ), context_->device( ).physical( ), image_size,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer_,
                              staging_buffer_memory_ );

    void* data;
    vkMapMemory( context_->device( ).logical( ), staging_buffer_memory_, 0, image_size, 0, &data );
    memcpy( data, pixels, static_cast<size_t>( image_size ) );
    vkUnmapMemory( context_->device( ).logical( ), staging_buffer_memory_ );

    // Cleanup original pixel data
    stbi_image_free( pixels );

    texture_image_ptr_ = std::make_unique<Image>( context_->device( ), ImageCreateInfo{
                                                      .extent = {
                                                          static_cast<uint32_t>( tex_width ), static_cast<uint32_t>( tex_height )
                                                      },
                                                      .format = VK_FORMAT_R8G8B8A8_SRGB,
                                                      .tiling = VK_IMAGE_TILING_OPTIMAL,
                                                      .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                                      .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                      .aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT
                                                  } );
    vk_transition_image_layout( texture_image_ptr_->handle( ), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
    vk_copy_buffer_to_image( staging_buffer_, texture_image_ptr_->handle( ), static_cast<uint32_t>( tex_width ),
                             static_cast<uint32_t>( tex_height ) );
    vk_transition_image_layout( texture_image_ptr_->handle( ), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

    vkDestroyBuffer( context_->device( ).logical( ), staging_buffer_, nullptr );
    vkFreeMemory( context_->device( ).logical( ), staging_buffer_memory_, nullptr );
}


void TriangleApplication::vk_create_texture_sampler( )
{
    VkSamplerCreateInfo sampler_info{};

    // The magFilter and minFilter fields specify how to interpolate texels that are magnified or minified.
    sampler_info.sType     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;

    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties( context_->device( ).physical( ), &properties );

    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy    = properties.limits.maxSamplerAnisotropy;

    // samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable           = VK_FALSE;
    sampler_info.compareOp               = VK_COMPARE_OP_ALWAYS;

    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod     = 0.0f;
    sampler_info.maxLod     = 0.0f;

    validation::throw_on_bad_result(
        vkCreateSampler( context_->device( ).logical( ), &sampler_info, nullptr, &texture_sampler_ ),
        "Failed to create texture sampler!" );
}


void TriangleApplication::vk_transition_image_layout( VkImage const image, VkFormat const /* format  const*/,
                                                      VkImageLayout const old_layout,
                                                      VkImageLayout const new_layout ) const
{
    VkCommandBuffer const command_buffer = begin_single_time_commands( context_->device( ).logical( ), command_pool_ );

    VkImageMemoryBarrier barrier{};
    barrier.sType     = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if ( old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if ( old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout ==
              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if ( old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        source_stage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument( "Unsupported layout transition!" );
    }

    vkCmdPipelineBarrier(
        command_buffer,
        source_stage, destination_stage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier );

    end_single_time_commands( context_->device( ).logical( ), command_pool_, command_buffer,
                              context_->device( ).graphics_queue( ) );
}


void TriangleApplication::vk_copy_buffer_to_image( VkBuffer const buffer, VkImage const image, uint32_t const width,
                                                   uint32_t const height ) const
{
    VkCommandBuffer const command_buffer = begin_single_time_commands( context_->device( ).logical( ), command_pool_ );

    VkBufferImageCopy region{};
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        command_buffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    end_single_time_commands( context_->device( ).logical( ), command_pool_, command_buffer,
                              context_->device( ).graphics_queue( ) );
}


void TriangleApplication::vk_create_index_buffer( )
{
    VkDeviceSize const buffer_size = sizeof( model_->get_indices( )[0] ) * model_->get_indices( ).size( );

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    Buffer::vk_create_buffer( context_->device( ).logical( ), context_->device( ).physical( ), buffer_size,
                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer,
                              staging_buffer_memory );

    void* data;
    vkMapMemory( context_->device( ).logical( ), staging_buffer_memory, 0, buffer_size, 0, &data );

    // ReSharper disable once CppRedundantCastExpression
    memcpy( data, model_->get_indices( ).data( ), static_cast<size_t>( buffer_size ) );

    vkUnmapMemory( context_->device( ).logical( ), staging_buffer_memory );

    Buffer::vk_create_buffer( context_->device( ).logical( ), context_->device( ).physical( ), buffer_size,
                              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer_, index_buffer_memory_ );

    Buffer::vk_copy_buffer( context_->device( ).logical( ), command_pool_, context_->device( ).graphics_queue( ),
                            staging_buffer, index_buffer_,
                            buffer_size );

    vkDestroyBuffer( context_->device( ).logical( ), staging_buffer, nullptr );
    vkFreeMemory( context_->device( ).logical( ), staging_buffer_memory, nullptr );
}


void TriangleApplication::vk_create_frame_buffers( )
{
    for ( size_t i = 0; i < swapchain_ptr_->images( ).size( ); i++ )
    {
        for ( auto const& image : swapchain_ptr_->images( ) )
        {
            swapchain_framebuffers_.emplace_back( context_->device( ), FramebufferCreateInfo{
                                                      .usage_type = FramebufferUsageType::RENDER_PASS,
                                                      .render_pass = render_pass_,
                                                      .extent = swapchain_ptr_->extent( ),
                                                      .attachments = {
                                                          image.view( ).handle( ), swapchain_depth_image_ptr_->view( ).handle( )
                                                      }
                                                  } );
        }
    }
}


void TriangleApplication::vk_create_uniform_buffers( )
{
    uniform_buffers_.resize( MAX_FRAMES_IN_FLIGHT_ );
    uniform_buffers_memory_.resize( MAX_FRAMES_IN_FLIGHT_ );
    uniform_buffers_mapped_.resize( MAX_FRAMES_IN_FLIGHT_ );

    // We map the buffer right after creation using vkMapMemory to get a pointer to which we can write the data later on.
    // The buffer stays mapped to this pointer for the application's whole lifetime. This technique is called
    // "persistent mapping" and works on all Vulkan implementations.
    for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        constexpr VkDeviceSize buffer_size = sizeof( UniformBufferObject );
        Buffer::vk_create_buffer( context_->device( ).logical( ), context_->device( ).physical( ), buffer_size,
                                  VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  uniform_buffers_[i], uniform_buffers_memory_[i] );

        vkMapMemory( context_->device( ).logical( ), uniform_buffers_memory_[i], 0, buffer_size, 0,
                     &uniform_buffers_mapped_[i] );
    }
}


void TriangleApplication::vk_create_descriptor_pool( )
{
    std::array<VkDescriptorPoolSize, 2> pool_sizes{};
    pool_sizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT_ );
    pool_sizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT_ );

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>( pool_sizes.size( ) );
    pool_info.pPoolSizes    = pool_sizes.data( );

    pool_info.maxSets = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT_ );

    validation::throw_on_bad_result(
        vkCreateDescriptorPool( context_->device( ).logical( ), &pool_info, nullptr, &descriptor_pool_ ),
        "Failed to create descriptor pool!" );
}


void TriangleApplication::vk_create_descriptor_sets( )
{
    std::vector const layouts( MAX_FRAMES_IN_FLIGHT_, descriptor_set_layout_ );
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool     = descriptor_pool_;
    alloc_info.descriptorSetCount = static_cast<uint32_t>( MAX_FRAMES_IN_FLIGHT_ );
    alloc_info.pSetLayouts        = layouts.data( );

    descriptor_sets_.resize( MAX_FRAMES_IN_FLIGHT_ );
    validation::throw_on_bad_result(
        vkAllocateDescriptorSets( context_->device( ).logical( ), &alloc_info, descriptor_sets_.data( ) ),
        "Failed to allocate descriptor sets!" );

    for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = uniform_buffers_[i];
        buffer_info.offset = 0;
        buffer_info.range  = sizeof( UniformBufferObject );

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView   = texture_image_ptr_->view( ).handle( );
        image_info.sampler     = texture_sampler_;

        std::array<VkWriteDescriptorSet, 2> descriptor_writes{};
        descriptor_writes[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet          = descriptor_sets_[i];
        descriptor_writes[0].dstBinding      = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo     = &buffer_info;

        descriptor_writes[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet          = descriptor_sets_[i];
        descriptor_writes[1].dstBinding      = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo      = &image_info;

        vkUpdateDescriptorSets( context_->device( ).logical( ), static_cast<uint32_t>( descriptor_writes.size( ) ),
                                descriptor_writes.data( ), 0, nullptr );
    }
}


void TriangleApplication::vk_create_command_pool( )
{
    auto const queue_family_indices = query::find_queue_families( context_->device( ).physical( ),
                                                                  context_->instance( ) );

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    // There are two possible flags for command pools:
    // 1. VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are rerecorded with new commands very often.
    // 2. VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command buffers to be rerecorded individually, without
    // this flag they all have to be reset together.
    // We will be recording a command buffer every frame, so we want to be able to reset and rerecord over it.
    pool_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value( );

    validation::throw_on_bad_result( vkCreateCommandPool( context_->device( ).logical( ), &pool_info, nullptr, &command_pool_ ),
                                     "Failed to create command pool!" );
}


void TriangleApplication::vk_create_command_buffers( )
{
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool_;

    // The level parameter specifies if the allocated command buffers are primary or secondary command buffers.
    // - VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other
    // command buffers.
    // - VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = static_cast<uint32_t>( command_buffers_.size( ) );

    validation::throw_on_bad_result(
        vkAllocateCommandBuffers( context_->device( ).logical( ), &alloc_info, command_buffers_.data( ) ),
        "Failed to allocate command buffers!" );
}


void TriangleApplication::vk_create_sync_objects( )
{
    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    // The VK_FENCE_CREATE_SIGNALED_BIT flag specifies that the fence object is created in the signaled state.
    // This allows us to skip the first render frame, or we would otherwise be stuck waiting at the start.
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        validation::throw_on_bad_result(
            vkCreateSemaphore( context_->device( ).logical( ), &semaphore_info, nullptr, &image_available_semaphores_[i] ),
            "Failed to create image semaphore!" );
        validation::throw_on_bad_result(
            vkCreateSemaphore( context_->device( ).logical( ), &semaphore_info, nullptr, &render_finished_semaphores_[i] ),
            "Failed to create render semaphore!" );
        validation::throw_on_bad_result(
            vkCreateFence( context_->device( ).logical( ), &fence_info, nullptr, &in_flight_fences_[i] ),
            "Failed to create fence!" );
    }
}


void TriangleApplication::vk_create_depth_resources( )
{
    VkFormat const depth_format = query::find_depth_format( context_->device( ).physical( ) );
    swapchain_depth_image_ptr_  = std::make_unique<Image>( context_->device( ), ImageCreateInfo{
                                                               .extent = swapchain_ptr_->extent( ),
                                                               .format = depth_format,
                                                               .tiling = VK_IMAGE_TILING_OPTIMAL,
                                                               .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                               .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                               .aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT
                                                           } );
    // vk_transition_image_layout( swapchain_depth_image_ptr_->handle( ), depth_format, VK_IMAGE_LAYOUT_UNDEFINED,
    //                             VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL );
}


VkBool32 TriangleApplication::debug_callback( VkDebugUtilsMessageSeverityFlagBitsEXT const message_severity,
                                              VkDebugUtilsMessageTypeFlagsEXT /* messageType */,
                                              VkDebugUtilsMessengerCallbackDataEXT const* p_callback_data,
                                              void* /* pUserData */ )
{
    std::string severity_color{};

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    constexpr std::string_view reset_color{ "\e[0m" };
#pragma GCC diagnostic pop
#else
#pragma warning(push, 0)
    constexpr std::string_view reset_color{ "\e[0m" };
#pragma warning(pop)
#endif

    switch ( message_severity )
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            severity_color = "\033[34m";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            severity_color = "\033[37m";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            severity_color = "\033[33m";
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            severity_color = "\033[31m";
            break;
        default:
            severity_color = "\033[36m";
            break;
    }

    std::cerr << severity_color << "validation layer: " << p_callback_data->pMessage << reset_color << std::endl;
    return VK_FALSE;
}


void TriangleApplication::record_command_buffer( VkCommandBuffer command_buffer, // NOLINT( const*-misplaced-const)
                                                 uint32_t const image_index ) const
{
    // We always begin recording a command buffer by calling vkBeginCommandBuffer with a small VkCommandBufferBeginInfo
    // structure as argument that specifies some details about the usage of this specific command buffer.
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // The flags parameter specifies how we're going to use the command buffer. The following values are available:
    // - VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it
    // once.
    // - VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely
    // within a single render pass.
    // - VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already
    // pending execution.
    // None of the flags are applicable for now.
    begin_info.flags            = 0;       // Optional
    begin_info.pInheritanceInfo = nullptr; // Optional

    validation::throw_on_bad_result( vkBeginCommandBuffer( command_buffer, &begin_info ),
                                     "Failed to begin recording command buffer!" );

    // Drawing starts by beginning the render pass with vkCmdBeginRenderPass. The render pass is configured using some
    // parameters in a VkRenderPassBeginInfo struct.
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass  = render_pass_;
    render_pass_info.framebuffer = swapchain_framebuffers_[image_index].handle( );

    render_pass_info.renderArea.offset = { 0, 0 };
    render_pass_info.renderArea.extent = swapchain_ptr_->extent( );

    // The range of depths in the depth buffer is 0.0 to 1.0 in Vulkan, where 1.0 lies at the far view plane and 0.0 at
    // the near view plane.
    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color        = { { 0.0f, 0.0f, 0.0f, 1.0f } };
    clear_values[1].depthStencil = { 1.0f, 0 };

    render_pass_info.clearValueCount = static_cast<uint32_t>( clear_values.size( ) );
    render_pass_info.pClearValues    = clear_values.data( );

    // The first parameter for every command is always the command buffer to record the command to. The second parameter
    // specifies the details of the render pass we've just provided. The final parameter controls how the drawing
    // commands within the render pass will be provided. It can have one of two values:
    // 1. VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself
    // and no secondary command buffers will be executed.
    // 2. VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass commands will be executed from secondary
    // command buffers.
    vkCmdBeginRenderPass( command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE );

    // We can now bind the graphics pipeline. The second parameter specifies if the pipeline object is a graphics or
    // compute pipeline.
    vkCmdBindPipeline( command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_ );

    // We did specify viewport and scissor state for this pipeline to be dynamic. So we need to set them in the command
    // buffer before issuing our draw command.
    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>( swapchain_ptr_->extent( ).width );
    viewport.height   = static_cast<float>( swapchain_ptr_->extent( ).height );
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport( command_buffer, 0, 1, &viewport );

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchain_ptr_->extent( );
    vkCmdSetScissor( command_buffer, 0, 1, &scissor );

    VkBuffer const vertex_buffers[]{ model_->get_vertex_buffer( ) };
    constexpr VkDeviceSize offsets[]{ 0 };
    vkCmdBindVertexBuffers( command_buffer, 0, 1, vertex_buffers, offsets );

    vkCmdBindIndexBuffer( command_buffer, index_buffer_, 0, VK_INDEX_TYPE_UINT32 );

    // Bind the right descriptor set for each frame to the descriptors in the shader.
    vkCmdBindDescriptorSets( command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1,
                             &descriptor_sets_[current_frame_], 0, nullptr );

    // We now issue the draw command. The number of indices represents the number of vertices that will be passed to
    // the vertex shader.
    vkCmdDrawIndexed( command_buffer, static_cast<uint32_t>( model_->get_indices( ).size( ) ), 1, 0, 0, 0 );

    // After we've finished recording the command buffer, we end the render pass with vkCmdEndRenderPass.
    vkCmdEndRenderPass( command_buffer );

    validation::throw_on_bad_result( vkEndCommandBuffer( command_buffer ), "Failed to record command buffer!" );
}


void TriangleApplication::update_uniform_buffer( uint32_t const current_image ) const
{
    static auto start_time = std::chrono::high_resolution_clock::now( );

    auto const current_time = std::chrono::high_resolution_clock::now( );
    float const time        = std::chrono::duration<float, std::chrono::seconds::period>( current_time - start_time ).count( );

    UniformBufferObject ubo{};
    ubo.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    ubo.view  = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ),
                             glm::vec3( 0.0f, 0.0f, 1.0f ) );
    ubo.proj = glm::perspective( glm::radians( 45.0f ),
                                 static_cast<float>( swapchain_ptr_->extent( ).width ) / static_cast<float>( swapchain_ptr_->
                                     extent( ).height ),
                                 0.1f,
                                 10.0f );
    ubo.proj[1][1] *= -1;

    memcpy( uniform_buffers_mapped_[current_image], &ubo, sizeof( ubo ) );
}


void TriangleApplication::draw_frame( )
{
    // At a high level, rendering a frame in Vulkan consists of a common set of steps:
    // 1. Wait for the previous frame to finish.
    // 2. Acquire an image from the swap chain.
    // 3. Record a command buffer which draws the scene onto that image.
    // 4. Submit the recorded command buffer.
    // 5. Present the swap chain image.

    // A core design philosophy in Vulkan is that synchronization of execution on the GPU is explicit. The order of
    // operations is up to us to define using various synchronization primitives.

    // A semaphore is used to add order between queue operations. Queue operations refer to the work we submit to a
    // queue, either in a command buffer or from within a function. The queueing happens on the CPU, but the execution
    // and the waiting happen on the GPU.

    // A fence has a similar purpose, in that it is used to synchronize execution, but it is for ordering the execution
    // on the CPU. Simply put, if the host needs to know when the GPU has finished something, we use a fence.

    // In summary, semaphores are used to specify the execution order of operations on the GPU while fences are used to
    // keep the CPU and GPU in sync with each-other.

    // We'll need one semaphore to signal that an image has been acquired from the swapchain and is ready for rendering,
    // another one to signal that rendering has finished and presentation can happen, and a fence to make sure only one
    // frame is rendering at a time.

    // The VK_TRUE we pass here indicates that we want to wait for all fences, but in the case of a single one it
    // doesn't matter. This function also has a timeout parameter set to UINT64_MAX, which effectively disables the
    // timeout.
    vkWaitForFences( context_->device( ).logical( ), 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX );

    // The first two parameters of vkAcquireNextImageKHR are the logical device and the swap chain from which we wish
    // to acquire an image.
    uint32_t image_index{};
    auto const acquire_image_result = vkAcquireNextImageKHR( context_->device( ).logical( ), swapchain_ptr_->handle( ),
                                                             UINT64_MAX,
                                                             image_available_semaphores_[current_frame_],
                                                             VK_NULL_HANDLE,
                                                             &image_index );

    // If viewport changes, recreate the swap chain.
    // - VK_ERROR_OUT_OF_DATE_KHR: The swap chain has become incompatible with the surface and can no longer be used for
    // rendering. Usually happens after a window resize.
    // - VK_SUBOPTIMAL_KHR: The swap chain can still be used to successfully present to the surface, but the surface
    // properties are no longer matched exactly.
    if ( acquire_image_result == VK_ERROR_OUT_OF_DATE_KHR )
    {
        vk_recreate_swap_chain( );
        return;
    }
    validation::throw_on_bad_result( acquire_image_result, "Failed to acquire swap chain image!", { VK_SUBOPTIMAL_KHR } );

    // After waiting, we need to manually reset the fence to the unsignaled state with the vkResetFences call.
    // We reset the fence only if there's work to do, which is why we're doing it after the resize check.
    // You should not return before work is completed or DEADLOCK will occur.
    vkResetFences( context_->device( ).logical( ), 1, &in_flight_fences_[current_frame_] );

    // With the imageIndex specifying the swap chain image to use in hand, we can now record the command buffer.
    vkResetCommandBuffer( command_buffers_[current_frame_], 0 );
    record_command_buffer( command_buffers_[current_frame_], image_index );

    // We need to submit the recorded command buffer to the graphics queue before submitting the image to the swap chain.
    update_uniform_buffer( current_frame_ );

    // Queue submission and synchronization is configured through parameters in the VkSubmitInfo structure.
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    constexpr VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submit_info.waitSemaphoreCount               = 1;
    submit_info.pWaitSemaphores                  = &image_available_semaphores_[current_frame_];
    submit_info.pWaitDstStageMask                = wait_stages;

    // Set which command buffers to actually submit for execution.
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &command_buffers_[current_frame_];

    // The signalSemaphoreCount and pSignalSemaphores parameters specify which semaphores to signal once the command
    // buffer(s) have finished execution
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = &render_finished_semaphores_[current_frame_];

    // The last parameter references an optional fence that will be signaled when the command buffers finish execution.
    // This allows us to know when it is safe for the command buffer to be reused, thus we want to give it inFlightFence.
    validation::throw_on_bad_result(
        vkQueueSubmit( context_->device( ).graphics_queue( ), 1, &submit_info, in_flight_fences_[current_frame_] ),
        "Failed to submit draw command buffer!" );

    // The last step of drawing a frame is submitting the result back to the swap chain to have it eventually show up
    // on the screen. Presentation is configured through a VkPresentInfoKHR structure.
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // These two parameters specify which semaphores to wait on before presentation can happen.
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = &render_finished_semaphores_[current_frame_];

    // These two parameters specify the swap chains to present images to and the index of the image for each swap chain.
    present_info.swapchainCount = 1;
    present_info.pSwapchains    = swapchain_ptr_->phandle( );
    present_info.pImageIndices  = &image_index;

    // There is one last optional parameter called pResults. It allows you to specify an array of VkResult values to
    // check for every individual swap chain if presentation was successful.
    present_info.pResults = nullptr; // Optional

    // The vkQueuePresentKHR function submits the request to present an image to the queue.

    // We check for the callback boolean after the queue presentation to avoid the semaphore to be signaled.
    if ( auto const queue_present_result = vkQueuePresentKHR( context_->device( ).graphics_queue( ), &present_info );
        queue_present_result == VK_ERROR_OUT_OF_DATE_KHR || queue_present_result == VK_SUBOPTIMAL_KHR ||
        frame_buffer_resized_ )
    {
        frame_buffer_resized_ = false;
        vk_recreate_swap_chain( );
    }

    // Next frame
    current_frame_ = ( current_frame_ + 1 ) % MAX_FRAMES_IN_FLIGHT_;
}


void TriangleApplication::init_vk( )
{
    swapchain_ptr_ = std::make_unique<Swapchain>( *context_, *window_, SwapchainCreateInfo{
                                                      .image_count = 3,
                                                      .present_mode = VK_PRESENT_MODE_MAILBOX_KHR,
                                                      .surface_format = {
                                                          VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
                                                      }
                                                  } );

    vk_create_render_pass( );

    vk_create_descriptor_set_layout( );

    vk_create_graphics_pipeline( );

    vk_create_command_pool( );

    vk_create_depth_resources( );

    vk_create_frame_buffers( );

    vk_create_texture_image( );
    vk_create_texture_sampler( );

    load_model( );
    vk_create_index_buffer( );
    vk_create_uniform_buffers( );

    vk_create_descriptor_pool( );
    vk_create_descriptor_sets( );

    vk_create_command_buffers( );

    vk_create_sync_objects( );
}


void TriangleApplication::configure_relative_path( ) const
{
    xos::info::log_info( std::clog );
    xos::filesystem::configure_relative_path( );
}


void TriangleApplication::main_loop( )
{
    glfwPollEvents( );

    draw_frame( );
}


void TriangleApplication::cleanup( )
{
    // Allocation and de-allocation functions in Vulkan have an optional allocator callback
    // that we'll ignore by passing nullptr.
    for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        vkDestroySemaphore( context_->device( ).logical( ), image_available_semaphores_[i], nullptr );
        vkDestroySemaphore( context_->device( ).logical( ), render_finished_semaphores_[i], nullptr );
        vkDestroyFence( context_->device( ).logical( ), in_flight_fences_[i], nullptr );
    }

    vkDestroyCommandPool( context_->device( ).logical( ), command_pool_, nullptr );

    vkDestroyPipeline( context_->device( ).logical( ), graphics_pipeline_, nullptr );
    vkDestroyPipelineLayout( context_->device( ).logical( ), pipeline_layout_, nullptr );

    vkDestroyRenderPass( context_->device( ).logical( ), render_pass_, nullptr );

    // vk_cleanup_swap_chain( );
    swapchain_framebuffers_.clear( );
    swapchain_depth_image_ptr_.reset( );
    swapchain_ptr_.reset( );

    vkDestroySampler( context_->device( ).logical( ), texture_sampler_, nullptr );
    texture_image_ptr_.reset( );

    for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT_; i++ )
    {
        vkDestroyBuffer( context_->device( ).logical( ), uniform_buffers_[i], nullptr );
        vkFreeMemory( context_->device( ).logical( ), uniform_buffers_memory_[i], nullptr );
    }

    vkDestroyDescriptorPool( context_->device( ).logical( ), descriptor_pool_, nullptr );
    vkDestroyDescriptorSetLayout( context_->device( ).logical( ), descriptor_set_layout_, nullptr );

    vkDestroyBuffer( context_->device( ).logical( ), index_buffer_, nullptr );
    vkFreeMemory( context_->device( ).logical( ), index_buffer_memory_, nullptr );

    CVK.reset_instance( );
}
