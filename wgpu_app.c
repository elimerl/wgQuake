#include "wgpu_app.h"
#include "SDL_events.h"
#include "sdl2webgpu.h"
#include "wgpu_utils.h"
#include <stdio.h>
#include <webgpu/webgpu.h>
#if defined(WEBGPU_BACKEND_WGPU)
#include <webgpu/wgpu.h>
#endif
WG app;

WG *WG_Instance() { return &app; }

void onDeviceError(WGPUErrorType type, char const *message, void *userData) {
  fprintf(stderr, "Uncaptured device error: type %i", type);
  if (message)
    fprintf(stderr, "(%s)", message);
  fprintf(stderr, "\n");
}

int WG_Init() {
  // Init WebGPU
  WGPUInstanceDescriptor desc = {.nextInChain = NULL};
  app.instance = wgpuCreateInstance(&desc);

  // Init SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "Could not init SDL!");
    return 1;
  }
  app.window = SDL_CreateWindow("wgQuake", SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);

  if (!app.window) {
    fprintf(stderr, "Could not init window!");
    return 1;
  }

  app.surface = SDL_GetWGPUSurface(app.instance, app.window);

  WGPURequestAdapterOptions adapterOpt = {
      .compatibleSurface = app.surface,
      .powerPreference = WGPUPowerPreference_HighPerformance};

  app.adapter = requestAdapterSync(app.instance, &adapterOpt);
  if (!app.adapter) {
    fprintf(stderr, "Could not init WebGPU adapter!");
    return 1;
  }

  WGPUDeviceDescriptor deviceDesc = {};
  app.device = requestDeviceSync(app.adapter, &deviceDesc);

  if (!app.device) {
    fprintf(stderr, "Could not init WebGPU device!");
    return 1;
  }
  wgpuDeviceSetUncapturedErrorCallback(app.device, onDeviceError, NULL);

  app.queue = wgpuDeviceGetQueue(app.device);

  int wwidth, wheight;
  SDL_GetWindowSize(app.window, &wwidth, &wheight);
  app.surfaceFormat = wgpuSurfaceGetPreferredFormat(app.surface, app.adapter);
  WGPUSurfaceConfiguration config = {.nextInChain = NULL,
                                     .width = wwidth,
                                     .height = wheight,
                                     .usage = WGPUTextureUsage_RenderAttachment,
                                     .format = app.surfaceFormat,
                                     .viewFormatCount = 0,
                                     .viewFormats = NULL,
                                     .device = app.device,
                                     .presentMode = WGPUPresentMode_Fifo,
                                     .alphaMode = WGPUCompositeAlphaMode_Auto};
  wgpuSurfaceConfigure(app.surface, &config);

  return 0;
}

void WG_Destroy() {
  wgpuQueueRelease(app.queue);
  wgpuDeviceRelease(app.device);
  wgpuAdapterRelease(app.adapter);
  wgpuInstanceRelease(app.instance);

  wgpuSurfaceRelease(app.surface);

  // Terminate SDL
  SDL_DestroyWindow(app.window);
  SDL_Quit();
}

WGPUTextureView WG_NextSurfaceTextureView() {
  WGPUSurfaceTexture surfaceTexture;
  wgpuSurfaceGetCurrentTexture(app.surface, &surfaceTexture);
  if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_Success) {
    return NULL;
  }
  WGPUTextureViewDescriptor viewDescriptor = {
      .nextInChain = NULL,
      .label = NULL,
      .format = wgpuTextureGetFormat(surfaceTexture.texture),
      .dimension = WGPUTextureViewDimension_2D,
      .baseMipLevel = 0,
      .mipLevelCount = 1,
      .baseArrayLayer = 0,
      .arrayLayerCount = 1,
      .aspect = WGPUTextureAspect_All,
  };

  WGPUTextureView view =
      wgpuTextureCreateView(surfaceTexture.texture, &viewDescriptor);

  return view;
}

WGPUShaderModule WG_CreateShaderModule(const char *shaderSource) {
  WGPUShaderModuleWGSLDescriptor shaderCodeDesc = {
      .code = shaderSource,
      .chain = {NULL, WGPUSType_ShaderModuleWGSLDescriptor}};
  WGPUShaderModuleDescriptor shaderDesc = {
      .label = NULL, .nextInChain = &shaderCodeDesc.chain};
#ifdef WEBGPU_BACKEND_WGPU
  shaderDesc.hintCount = 0;
  shaderDesc.hints = NULL;
#endif
  return wgpuDeviceCreateShaderModule(app.device, &shaderDesc);
}

WGPURenderPipeline WG_CreateRenderPipeline(WGPUShaderModule shader,
                                           const char *vertexEntryPoint,
                                           const char *fragEntryPoint) {
  WGPUVertexState vertexState = {.nextInChain = NULL,
                                 .bufferCount = 0,
                                 .buffers = NULL,
                                 .module = shader,
                                 .entryPoint = vertexEntryPoint,
                                 .constantCount = 0,
                                 .constants = NULL};

  WGPUBlendState blendState = {
      .color = {WGPUBlendOperation_Add, WGPUBlendFactor_SrcAlpha,
                WGPUBlendFactor_OneMinusSrcAlpha},
      .alpha = {WGPUBlendOperation_Add, WGPUBlendFactor_Zero,
                WGPUBlendFactor_One}};

  WGPUColorTargetState colorTarget = {.format = app.surfaceFormat,
                                      .blend = &blendState,
                                      .writeMask = WGPUColorWriteMask_All};

  WGPUFragmentState fragmentState = {.nextInChain = NULL,
                                     .module = shader,
                                     .entryPoint = fragEntryPoint,
                                     .constantCount = 0,
                                     .constants = NULL,
                                     .targetCount = 1,
                                     .targets = &colorTarget};
  WGPURenderPipelineDescriptor desc = {
      .nextInChain = NULL,
      .label = NULL,

      .vertex = vertexState,
      .fragment = &fragmentState,
      .primitive = {.topology = WGPUPrimitiveTopology_TriangleList,
                    .stripIndexFormat = WGPUIndexFormat_Undefined,
                    .frontFace = WGPUFrontFace_CCW,
                    .cullMode = WGPUCullMode_Back},
      .layout = NULL,
      .multisample = {NULL, 1, ~0u, false},
      .depthStencil = NULL,
  };

  return wgpuDeviceCreateRenderPipeline(app.device, &desc);
}
