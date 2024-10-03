#ifndef _WGPU_APP_H
#define _WGPU_APP_H

#include <SDL2/SDL.h>
#include <webgpu/webgpu.h>

#include <stdbool.h>

typedef struct {
  SDL_Window *window;

  WGPUInstance instance;
  WGPUAdapter adapter;
  WGPUDevice device;
  WGPUQueue queue;

  WGPUSurface surface;

  bool shouldStop;
} RenderApp;

RenderApp *RenderApp_Instance();

int RenderApp_Init();
int RenderApp_Tick();
void RenderApp_Destroy();

WGPUTextureView RenderApp_NextSurfaceTextureView();


#endif