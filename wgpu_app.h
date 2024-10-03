#ifndef _WGPU_APP_H
#define _WGPU_APP_H

#include <SDL2/SDL.h>
#include <webgpu/webgpu.h>

typedef struct {
  SDL_Window *window;

  WGPUInstance instance;
  WGPUAdapter adapter;
  WGPUDevice device;
  WGPUQueue queue;

  WGPUSurface surface;
} RenderAppData;

int RenderApp_Init();
WGPUTextureView RenderApp_NextSurfaceTextureView();

#endif