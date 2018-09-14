/*
 * Vulkan Samples
 *
 * Copyright (C) 2015-2016 Valve Corporation
 * Copyright (C) 2015-2016 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
VULKAN_SAMPLE_SHORT_DESCRIPTION
Draw Cube
*/

/* This is part of the draw cube progression */

#include <util_init.hpp>
#include <assert.h>
#include <string.h>
#include <cstdlib>
#include "cube_data.h"

/* For this sample, we'll start with GLSL so the shader function is plain */
/* and then use the glslang GLSLtoSPV utility to convert it to SPIR-V for */
/* the driver.  We do this for clarity rather than using pre-compiled     */
/* SPIR-V                                                                 */

static const char *vertShaderText =
    "#version 450 core\n"
    "struct ANGLEDepthRangeParams {\n"
    "float near;\n"
    "float far;\n"
    "float diff;\n"
    "float dummyPacker;\n"
    "};\n"
    "layout(location = 0) in vec3 _ua_positionSize;\n"
    "layout(set = 2, binding = 0) uniform ANGLEUniformBlock {\n"
    "    vec4 viewport;\n"
    "    vec4 viewportScaleFactor;\n"
    "    ANGLEDepthRangeParams depthRange;\n"
    "}\n"
    "ANGLEUniforms;\n"
    "void main() {\n"
    "    (gl_Position = vec4(_ua_positionSize.xy, 0.0, 1.0));\n"
    "    (gl_PointSize = _ua_positionSize.z);\n"
    "    (gl_Position.z = (gl_Position.w * ((gl_Position.z * 0.5) + 0.5)));\n"
    "}\n";
//"#version 400\n"
//"#extension GL_ARB_separate_shader_objects : enable\n"
//"#extension GL_ARB_shading_language_420pack : enable\n"
//"layout (std140, binding = 0) uniform bufferVals {\n"
//"    mat4 mvp;\n"
//"} myBufferVals;\n"
//"layout (location = 0) in vec4 pos;\n"
//"layout (location = 1) in vec4 inColor;\n"
//"layout (location = 0) out vec4 outColor;\n"
//"void main() {\n"
//"   outColor = inColor;\n"
//"   gl_Position = myBufferVals.mvp * pos;\n"
//"}\n";

static const char *fragShaderText =
    "#version 450 core\n"
    "        layout(location = 0) out vec4 webgl_FragColor;\n"
    "vec2 _uflippedPointCoord;\n"
    "struct ANGLEDepthRangeParams {\n"
    "    float near;\n"
    "    float far;\n"
    "    float diff;\n"
    "    float dummyPacker;\n"
    "};\n"
#if USE_PUSH_CONSTANTS
    "layout(push_constant) uniform ANGLEUniformBlock {\n"
    "    vec4 viewport;\n"
    "    vec4 viewportScaleFactor;\n"
    "    ANGLEDepthRangeParams depthRange;\n"
    "}\n"
#else
    "layout(set = 2, binding = 0) uniform ANGLEUniformBlock {\n"
    "    vec4 viewport;\n"
    "    vec4 viewportScaleFactor;\n"
    "    ANGLEDepthRangeParams depthRange;\n"
    "}\n"
#endif
    "ANGLEUniforms;\n"
    "void main() {\n"
    "    _uflippedPointCoord = vec2(gl_PointCoord.x, (((gl_PointCoord.y + -0.5) * ANGLEUniforms.viewportScaleFactor.y) + "
    //"    _uflippedPointCoord = vec2(gl_PointCoord.x, (((gl_PointCoord.y + -0.5) * 1.0) + "
    "0.5));\n"
    "    (webgl_FragColor = vec4(_uflippedPointCoord, 0.0, 1.0));\n"
    "}\n";
//"#version 400\n"
//"#extension GL_ARB_separate_shader_objects : enable\n"
//"#extension GL_ARB_shading_language_420pack : enable\n"
//"layout (location = 0) in vec4 color;\n"
//"layout (location = 0) out vec4 outColor;\n"
//"void main() {\n"
//"   outColor = color;\n"
//"}\n";

int sample_main(int argc, char *argv[]) {
    VkResult U_ASSERT_ONLY res;
    struct sample_info info = {};
    char sample_title[] = "Draw Cube";
    const bool depthPresent = true;

    process_command_line_args(info, argc, argv);
    init_global_layer_properties(info);
    init_instance_extension_names(info);
    init_device_extension_names(info);
    init_instance(info, sample_title);
    init_enumerate_device(info);
    init_window_size(info, 400, 300);
    init_connection(info);
    init_window(info);
    init_swapchain_extension(info);
    init_device(info);

    init_command_pool(info);
    init_command_buffer(info);
    execute_begin_command_buffer(info);
    init_device_queue(info);
    init_swap_chain(info);
    init_depth_buffer(info);
    init_uniform_buffer(info);
    init_descriptor_and_pipeline_layouts(info, false);
    init_renderpass(info, depthPresent);
    init_shaders(info, vertShaderText, fragShaderText);
    init_framebuffers(info, depthPresent);
    // TODO: Init vtx buffers as ANGLE does
    //    init_vertex_buffer(info, g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data),
    //                       sizeof(g_vb_solid_face_colors_Data[0]), false);
    // TODO: Just throwing data from ANGLE into byte array for now
    unsigned char vtx_array[96] = {};
    init_vtx_array(vtx_array);
    init_vertex_buffer(info, vtx_array, sizeof(vtx_array), 12, false);
    init_descriptor_pool(info, false);
    init_descriptor_set(info, false);
    init_pipeline_cache(info);
    //    init_pipeline(info, depthPresent);
    init_gfx_pipeline(info, depthPresent);

    /* VULKAN_KEY_START */

    VkClearValue clear_values[2];
    clear_values[0].color.float32[0] = 0.0f;
    clear_values[0].color.float32[1] = 0.0f;
    clear_values[0].color.float32[2] = 0.0f;
    clear_values[0].color.float32[3] = 1.0f;
    clear_values[0].depthStencil.depth = 0.0f;
    clear_values[0].depthStencil.stencil = 0;
    clear_values[1].color.float32[0] = 1.0f;
    clear_values[1].color.float32[1] = 0.0f;
    clear_values[1].color.float32[2] = 0.0f;
    clear_values[1].color.float32[3] = 0.0f;
    clear_values[1].depthStencil.depth = 1.0f;
    clear_values[1].depthStencil.stencil = 0;

    VkSemaphore imageAcquiredSemaphore;
    VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
    imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    imageAcquiredSemaphoreCreateInfo.pNext = NULL;
    imageAcquiredSemaphoreCreateInfo.flags = 0;

    res = vkCreateSemaphore(info.device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
    assert(res == VK_SUCCESS);

    // Get the index of the next available swapchain image:
    res = vkAcquireNextImageKHR(info.device, info.swap_chain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE,
                                &info.current_buffer);
    // TODO: Deal with the VK_SUBOPTIMAL_KHR and VK_ERROR_OUT_OF_DATE_KHR
    // return codes
    assert(res == VK_SUCCESS);

    VkRenderPassBeginInfo rp_begin;
    rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.pNext = NULL;
    rp_begin.renderPass = info.render_pass;
    rp_begin.framebuffer = info.framebuffers[info.current_buffer];
    rp_begin.renderArea.offset.x = 0;
    rp_begin.renderArea.offset.y = 0;
    rp_begin.renderArea.extent.width = info.width;
    rp_begin.renderArea.extent.height = info.height;
    rp_begin.clearValueCount = 2;
    rp_begin.pClearValues = clear_values;

    vkCmdBeginRenderPass(info.cmd, &rp_begin, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    // TODO: Need a secondary cmd buffer that I put these commands in
#if USE_PUSH_CONSTANTS
    float push_constants[12] = {3.0f, 43.0f, 256.0f, 256.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f};
    vkCmdPushConstants(info.cmd, info.pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push_constants), push_constants);
#endif

    vkCmdBindPipeline(info.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, info.pipeline);
    //vkCmdBindDescriptorSets(info.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, info.pipeline_layout, 0, 1, &info.desc_set[2], 0, NULL);
    vkCmdBindDescriptorSets(info.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, info.pipeline_layout, 2, 1, &info.desc_set[2], 0, NULL);

    const VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(info.cmd, 0, 1, &info.vertex_buffer.buf, offsets);
    // Create a memory barrier here to make sure uniform data available to GPU
    VkBufferMemoryBarrier buffer_barrier = {};
    buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    buffer_barrier.pNext = nullptr;
    buffer_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
    buffer_barrier.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
    buffer_barrier.srcQueueFamilyIndex = 0;
    buffer_barrier.dstQueueFamilyIndex = 0;
    buffer_barrier.buffer = info.uniform_data.buf;
    buffer_barrier.offset = 0;
    buffer_barrier.size = 256;

    vkCmdPipelineBarrier(info.cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 1,
                         &buffer_barrier, 0, nullptr);

    init_viewports(info);
    init_scissors(info);

    // vkCmdDraw(info.cmd, 12 * 3, 1, 0, 0);
    vkCmdDraw(info.cmd, 8, 1, 0, 0);
    vkCmdEndRenderPass(info.cmd);
    res = vkEndCommandBuffer(info.cmd);
    const VkCommandBuffer cmd_bufs[] = {info.cmd};
    VkFenceCreateInfo fenceInfo;
    VkFence drawFence;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;
    fenceInfo.flags = 0;
    vkCreateFence(info.device, &fenceInfo, NULL, &drawFence);

    VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info[1] = {};
    submit_info[0].pNext = NULL;
    submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info[0].waitSemaphoreCount = 1;
    submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;
    submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
    submit_info[0].commandBufferCount = 1;
    submit_info[0].pCommandBuffers = cmd_bufs;
    submit_info[0].signalSemaphoreCount = 0;
    submit_info[0].pSignalSemaphores = NULL;

    /* Queue the command buffer for execution */
    res = vkQueueSubmit(info.graphics_queue, 1, submit_info, drawFence);
    assert(res == VK_SUCCESS);

    /* Now present the image in the window */

    VkPresentInfoKHR present;
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pNext = NULL;
    present.swapchainCount = 1;
    present.pSwapchains = &info.swap_chain;
    present.pImageIndices = &info.current_buffer;
    present.pWaitSemaphores = NULL;
    present.waitSemaphoreCount = 0;
    present.pResults = NULL;

    /* Make sure command buffer is finished before presenting */
    do {
        res = vkWaitForFences(info.device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
    } while (res == VK_TIMEOUT);

    assert(res == VK_SUCCESS);
    res = vkQueuePresentKHR(info.present_queue, &present);
    assert(res == VK_SUCCESS);

    wait_seconds(10);
    /* VULKAN_KEY_END */
    if (info.save_images) write_ppm(info, "15-draw_cube");

    vkDestroySemaphore(info.device, imageAcquiredSemaphore, NULL);
    vkDestroyFence(info.device, drawFence, NULL);
    destroy_pipeline(info);
    destroy_pipeline_cache(info);
    destroy_descriptor_pool(info);
    destroy_vertex_buffer(info);
    destroy_framebuffers(info);
    destroy_shaders(info);
    destroy_renderpass(info);
    destroy_descriptor_and_pipeline_layouts(info);
    destroy_uniform_buffer(info);
    destroy_depth_buffer(info);
    destroy_swap_chain(info);
    destroy_command_buffer(info);
    destroy_command_pool(info);
    destroy_device(info);
    destroy_window(info);
    destroy_instance(info);
    return 0;
}
