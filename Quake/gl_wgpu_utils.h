#ifndef _GL_WGPU_UTILS_H
#define _GL_WGPU_UTILS_H

#include <SDL2/SDL.h>
#include <webgpu/webgpu.h>

#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif
typedef struct {
  SDL_Window *window;

  WGPUInstance instance;
  WGPUAdapter adapter;
  WGPUDevice device;
  WGPUQueue queue;

  WGPUSurface surface;

  WGPUTextureFormat surfaceFormat;
} WG;

WGPUAdapter requestAdapterSync(WGPUInstance instance,
                               WGPURequestAdapterOptions const *options);
WGPUDevice requestDeviceSync(WGPUAdapter adapter,
                             WGPUDeviceDescriptor const *descriptor);

static WG wgpu;

int WG_Init(SDL_Window *window);
void WG_Destroy();

WGPUTextureView WG_NextSurfaceTextureView();

WGPUShaderModule WG_CreateShaderModule(const char *shaderSource);

WGPURenderPipeline WG_CreateRenderPipeline(WGPUShaderModule shader,
                                           const char *vertexEntryPoint,
                                           const char *fragEntryPoint);
WGPUBuffer WG_CreateBuffer(WGPUBufferUsageFlags usage, uint64_t size);
void WG_WriteBuffer(WGPUBuffer buffer, int len, char *bytes);

#endif