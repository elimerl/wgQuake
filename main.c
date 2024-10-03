
#define SDL_MAIN_HANDLED
#include "sdl2webgpu.h"
#include <SDL2/SDL.h>
#include <webgpu/webgpu.h>
#if WEBGPU_BACKEND_WGPU
#include <webgpu/wgpu.h>
#endif

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include "wgpu_app.h"

int main(int argc, char *argv[]) {
  WG *app = WG_Instance();
  WG_Init();

  const char *shaderSource = "@vertex \
      fn vs_main(@builtin(vertex_index) in_vertex_index: u32) ->  \
      @builtin(position) vec4f { \
          var p = vec2f(0.0, 0.0); \
          if (in_vertex_index == 0u) { \
              p = vec2f(-0.5, -0.5); \
          } else if (in_vertex_index == 1u) { \
              p = vec2f(0.5, -0.5); \
          } else { \
              p = vec2f(0.0, 0.5); \
          } \
          return vec4f(p, 0.0, 1.0); \
      } \
       \
      @fragment \
      fn fs_main() -> @location(0) vec4f { \
          return vec4f(0.0, 1.0, 1.0, 1.0); \
      }";
  WGPUShaderModule module = WG_CreateShaderModule(shaderSource);
  assert(module);
  WGPURenderPipeline pipeline =
      WG_CreateRenderPipeline(module, "vs_main", "fs_main");
  wgpuShaderModuleRelease(module);

  while (!app->shouldStop) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        app->shouldStop = true;
      else if (event.type == SDL_WINDOWEVENT &&
               event.window.event == SDL_WINDOWEVENT_CLOSE)
        app->shouldStop = true;
    }

    WGPUTextureView targetView = WG_NextSurfaceTextureView();
    if (!targetView)
      break;

    WGPUCommandEncoderDescriptor encoderDesc = {};
    encoderDesc.nextInChain = NULL;
    WGPUCommandEncoder encoder =
        wgpuDeviceCreateCommandEncoder(app->device, &encoderDesc);

    WGPURenderPassColorAttachment renderPassColorAttachment = {
        .view = targetView,
        .resolveTarget = NULL,
        .loadOp = WGPULoadOp_Clear,
        .storeOp = WGPUStoreOp_Store,
        .clearValue = {1.0, 0.0, 0.0, 1.0}};

    WGPURenderPassDescriptor renderPassDesc = {.nextInChain = NULL,
                                               .colorAttachmentCount = 1,
                                               .colorAttachments =
                                                   &renderPassColorAttachment,
                                               .depthStencilAttachment = NULL,
                                               .timestampWrites = NULL};

    WGPURenderPassEncoder renderPass =
        wgpuCommandEncoderBeginRenderPass(encoder, &renderPassDesc);

    // draw
    wgpuRenderPassEncoderSetPipeline(renderPass, pipeline);
    // Draw 1 instance of a 3-vertices shape
    wgpuRenderPassEncoderDraw(renderPass, 3, 1, 0, 0);

    wgpuRenderPassEncoderEnd(renderPass);
    wgpuRenderPassEncoderRelease(renderPass);

    WGPUCommandBufferDescriptor cmdBufferDescriptor = {};
    cmdBufferDescriptor.nextInChain = NULL;
    WGPUCommandBuffer command =
        wgpuCommandEncoderFinish(encoder, &cmdBufferDescriptor);
    wgpuCommandEncoderRelease(encoder);

    wgpuQueueSubmit(app->queue, 1, &command);
    wgpuCommandBufferRelease(command);

    wgpuTextureViewRelease(targetView);

#ifndef __EMSCRIPTEN__
    wgpuSurfacePresent(app->surface);
#endif

#if defined(WEBGPU_BACKEND_DAWN)
    wgpuDeviceTick(device);
#elif defined(WEBGPU_BACKEND_WGPU)
    wgpuDevicePoll(app->device, false, NULL);
#endif
  }

  wgpuRenderPipelineRelease(pipeline);
  WG_Destroy();
  return 0;
}