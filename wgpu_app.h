// #ifndef _WGPU_APP_H
// #define _WGPU_APP_H

// #include <SDL2/SDL.h>
// #include <webgpu/webgpu.h>

// #include <stdbool.h>

// typedef struct {
//   SDL_Window *window;

//   WGPUInstance instance;
//   WGPUAdapter adapter;
//   WGPUDevice device;
//   WGPUQueue queue;

//   WGPUSurface surface;

//   WGPUTextureFormat surfaceFormat;

//   bool shouldStop;
// } WG;

// WG *WG_Instance();

// int WG_Init();
// void WG_Destroy();

// WGPUTextureView WG_NextSurfaceTextureView();

// WGPUShaderModule WG_CreateShaderModule(const char *shaderSource);

// WGPURenderPipeline WG_CreateRenderPipeline(WGPUShaderModule shader,
//                                            const char *vertexEntryPoint,
//                                            const char *fragEntryPoint);
// WGPUBuffer WG_CreateBuffer(WGPUBufferUsageFlags usage, uint64_t size);
// void WG_WriteBuffer(WGPUBuffer buffer, int len, char *bytes);

// #endif